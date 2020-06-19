#include "PAL/DescriptorHeap.h"

namespace
{
	const BRWL_CHAR* getErrorMessge(D3D12_DESCRIPTOR_HEAP_TYPE type)
	{
		switch (type) {
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			return BRWL_CHAR_LITERAL("Failed to create shader resource view descriptor heap.");
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			return BRWL_CHAR_LITERAL("Failed to create sampler descriptor heap.");
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			return BRWL_CHAR_LITERAL("Failed to create render target view descriptor heap.");
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			return BRWL_CHAR_LITERAL("Failed to create depth stencil view descriptor heap.");
		default:
			BRWL_UNREACHABLE();
			return BRWL_CHAR_LITERAL("Failed to create descriptor heap of unknown/invalid type.");
		}
	}

	const BRWL_CHAR* getHeapName(D3D12_DESCRIPTOR_HEAP_TYPE type, bool isGpu)
	{
		switch (type) {
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			return isGpu ? BRWL_CHAR_LITERAL("SRV DESCR HEAP GPU") : BRWL_CHAR_LITERAL("SRV DESCR HEAP CPU");
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
			return isGpu ? BRWL_CHAR_LITERAL("SAMPLER HEAP GPU") : BRWL_CHAR_LITERAL("SAMPLER HEAP CPU");
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			return isGpu ? BRWL_CHAR_LITERAL("RTV HEAP GPU"): BRWL_CHAR_LITERAL("RTV HEAP CPU");
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			return isGpu ? BRWL_CHAR_LITERAL("DSV HEAP GPU") : BRWL_CHAR_LITERAL("DSV HEAP CPU");
		default:
			BRWL_UNREACHABLE();
			return BRWL_CHAR_LITERAL("Failed to create descriptor heap of unknown/invalid type.");
		}
	}
}

BRWL_RENDERER_NS

namespace PAL
{

	DescriptorHandle::DescriptorHandle() :
		nativeHandles{ {0}, {0} },
		owningHeap(nullptr),
		offset(0),
		count(0),
		name(nullptr),
		resident(false),
		remove(false)
	{ }

	DescriptorHandle::NonResidentHandles DescriptorHandle::getNonResident(int idx)
	{
		BRWL_EXCEPTION(count > 0 && owningHeap != nullptr, BRWL_CHAR_LITERAL("Accessing invalid descriptor"));
		BRWL_EXCEPTION(idx >= 0 && idx < count, BRWL_CHAR_LITERAL("Invalid descriptor access."));
		NonResidentHandles handles{ nativeHandles.cpu, nativeHandles.gpu };
		handles.cpu.ptr += owningHeap->descriptorSize * idx;
		handles.gpu.ptr += owningHeap->descriptorSize * idx;
		// TODO: IMPROVE THIS! THIS IS UGLY!
		// if we have been accessed on the cpu side, then we expect the data to maybe have changed
		owningHeap->dirtyArray[this - owningHeap->handles.data()] = true;
		return handles;
	}

	DescriptorHandle::ResidentHandles DescriptorHandle::getResident(int idx)
	{
		BRWL_EXCEPTION(count > 0 && owningHeap != nullptr, BRWL_CHAR_LITERAL("Accessing invalid descriptor"));
		BRWL_EXCEPTION(idx >= 0 && idx < count, BRWL_CHAR_LITERAL("Invalid descriptor access."));
		ResidentHandles handles{ nativeHandles.residentCpu, nativeHandles.residentGpu };
		handles.residentCpu.ptr += owningHeap->descriptorSize * idx;
		handles.residentGpu.ptr += owningHeap->descriptorSize * idx;
		return handles;
	}

	DescriptorHandle::NonResidentHandles DescriptorHandle::getNonResident() const
	{
		BRWL_EXCEPTION(count == 1, BRWL_CHAR_LITERAL("Invalid descriptor access."));
		// TODO: IMPROVE THIS! THIS IS UGLY!
		// if we have been accessed on the cpu side, then we expect the data to maybe have changed
		owningHeap->dirtyArray[this - owningHeap->handles.data()] = true;
		return { nativeHandles.cpu, nativeHandles.gpu };
	}

	DescriptorHandle::ResidentHandles DescriptorHandle::getResident() const
	{
		BRWL_EXCEPTION(count == 1, BRWL_CHAR_LITERAL("Invalid descriptor access."));
		return { nativeHandles.residentCpu, nativeHandles.residentGpu };
	}

	void DescriptorHandle::release()
	{
		if (!owningHeap) return;
		if (count == 1)
		{
			owningHeap->releaseOne(this);
		}
		else if (count > 1)
		{
			owningHeap->releaseRange(this);
		}
		else
		{
			BRWL_CHECK(false, BRWL_CHAR_LITERAL("Invalid descriptor handle release."));
		}
	}

	DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) :
		heapType(type),
		descriptorSize(0),
		numOccupiedDescriptors(0),
		cpuHeap(nullptr),
		gpuHeap(nullptr),
		queueCompletedIdx(0),
		queueNextIdx(0),
		dirtyArray(),
		staleQueue(),
		lastHandle(0),
		lastFree(0),
		handles(),
		cpuOccupied(),
		gpuOccupied(),
		created(false)
	{ }

	DescriptorHeap::~DescriptorHeap()
	{
		destroy();
	}

	bool DescriptorHeap::create(ID3D12Device* device, unsigned int capacity)
	{
		std::scoped_lock(mutex);
		BRWL_EXCEPTION(!created, nullptr);
		destroy();

		descriptorSize = device->GetDescriptorHandleIncrementSize(heapType);
		numOccupiedDescriptors = 0;
		{	// cpu heap
			D3D12_DESCRIPTOR_HEAP_DESC desc = { };
			desc.Type = heapType;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NumDescriptors = capacity;
			desc.NodeMask = 0;

			if (!BRWL_VERIFY(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&cpuHeap))), getErrorMessge(heapType)))
			{
				destroy();
				return false;
			}

			cpuHeap->SetName(getHeapName(heapType, false));
		}
		
		{	// gpu heap
			D3D12_DESCRIPTOR_HEAP_DESC desc = { };
			desc.Type = heapType;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			desc.NumDescriptors = capacity;
			desc.NodeMask = 0;

			if (!BRWL_VERIFY(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&gpuHeap))), getErrorMessge(heapType)))
			{
				destroy();
				return false;
			}

			gpuHeap->SetName(getHeapName(heapType, true));
		}
		
		handles.resize(capacity);
		cpuOccupied.resize(capacity);
		std::fill(cpuOccupied.begin(), cpuOccupied.end(), -1);
		gpuOccupied.resize(capacity);
		std::fill(gpuOccupied.begin(), gpuOccupied.end(), -1);
		dirtyArray.resize(capacity);

		created = true;

        return created;
	}

	void DescriptorHeap::destroy()
	{
		std::scoped_lock(mutex);
		created = false;
		descriptorSize = 0;
		numOccupiedDescriptors = 0;
		cpuHeap = nullptr;
		gpuHeap = nullptr;
		dirtyArray.clear();
		staleQueue.clear();
		handles.clear();
		cpuOccupied.clear();
		gpuOccupied.clear();
		lastHandle = 0;
		lastFree = 0;
	}

	DescriptorHandle* DescriptorHeap::allocateOne(const BRWL_CHAR* name, bool forceUpdate)
	{
		std::scoped_lock(mutex);

		if (!BRWL_VERIFY(numOccupiedDescriptors < handles.size(), BRWL_CHAR_LITERAL("Cannot allocate another descriptor handle. Heap full.")))
		{
			return nullptr;
		}

		// Find a new handle
		for (int i = 0; i < handles.size(); ++i)
		{
			lastHandle = (lastHandle + 1) % handles.size();
			if (handles[lastHandle].owningHeap == nullptr)
			{
				break;
			}
		}

		DescriptorHandle* handle = &handles[lastHandle];
		BRWL_CHECK(handle->owningHeap == nullptr, nullptr);
		BRWL_CHECK(handle->resident == false, nullptr);
		BRWL_CHECK(handle->remove == false, nullptr);
		BRWL_EXCEPTION(dirtyArray[lastHandle] == false, nullptr);

		// Find a free descriptor
		for (int i = 0; i < handles.size(); ++i)
		{
			lastFree = (lastFree + 1) % cpuOccupied.size();
			if (cpuOccupied[lastFree] == -1)
			{
				cpuOccupied[lastFree] = lastHandle;
				break;
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cpu = cpuHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu = cpuHeap->GetGPUDescriptorHandleForHeapStart();

		handle->owningHeap = this;
		handle->offset = lastFree;
		handle->count = 1;
		handle->name = name;
		handle->nativeHandles.cpu.ptr = cpu.ptr + descriptorSize * handle->offset;
		handle->nativeHandles.gpu.ptr = gpu.ptr + descriptorSize * handle->offset;
		handle->resident = false;
		
		dirtyArray[lastHandle] = true;
		
		++numOccupiedDescriptors;

		if (forceUpdate) {
			notifyNewFrameStarted();
			notifyOldFrameCompleted();
		}
		return handle;
	}

	void DescriptorHeap::releaseOne(DescriptorHandle* handle)
	{
		std::scoped_lock(mutex);

		BRWL_EXCEPTION(handle && handle->owningHeap == this, nullptr);
		size_t idx = handle - handles.data();
		BRWL_EXCEPTION(idx >= 0 && idx < handles.size(), nullptr);
		BRWL_EXCEPTION(handle->count == 1, nullptr);

		// mark as not available but don't free yet completely
		// the rest is done in notifyOldFrameCompleted possibly a few frames later
		handle->nativeHandles.cpu.ptr = handle->nativeHandles.gpu.ptr = 0;
		handle->remove = true;
		staleQueue.emplace_back(queueNextIdx, handle);
	}

	DescriptorHandle* DescriptorHeap::allocateRange(unsigned int n, const BRWL_CHAR* name)
	{
		std::scoped_lock(mutex);

		if (!BRWL_VERIFY(numOccupiedDescriptors  + n <= handles.size(), BRWL_CHAR_LITERAL("Cannot allocate requested descriptor range. Heap too full.")))
		{
			return nullptr;
		}

		for (int i = 0; i < handles.size(); ++i)
		{
			lastHandle = (lastHandle + 1) % handles.size();
			if (handles[lastHandle].owningHeap == nullptr)
				break;
		}

		DescriptorHandle* handle = &handles[lastHandle];
		BRWL_CHECK(handle->owningHeap == nullptr, nullptr);

		// after maintain we expect lastFree to have the index of the last occupied place in the cpu tracking array
		maintain();
		lastFree = (lastFree + 1) % handles.size();
		for (unsigned int i = 0; i < n; ++i)
		{
			BRWL_EXCEPTION(cpuOccupied[lastFree + i] == -1, nullptr);
			cpuOccupied[lastFree + i] = lastHandle;
		}
		
		dirtyArray[lastHandle] = true;

		D3D12_CPU_DESCRIPTOR_HANDLE cpu = cpuHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu = cpuHeap->GetGPUDescriptorHandleForHeapStart();

		handle->owningHeap = this;
		handle->offset = lastFree;
		handle->count = n;
		handle->name = name;
		handle->nativeHandles.cpu.ptr = cpu.ptr + descriptorSize * handle->offset;
		handle->nativeHandles.gpu.ptr = gpu.ptr + descriptorSize * handle->offset;
		handle->resident = false;
		lastFree = (lastFree + n - 1) % cpuOccupied.size();

		numOccupiedDescriptors += n;

		return handle;
	}

	void DescriptorHeap::releaseRange(DescriptorHandle* handle)
	{
		std::scoped_lock(mutex);

		BRWL_EXCEPTION(handle && handle->owningHeap == this, nullptr);
		size_t idx = handle - handles.data();
		BRWL_EXCEPTION(idx > 0 && idx + handle->count <= handles.size(), nullptr);
		BRWL_EXCEPTION(handle->count > 1, nullptr);

		// mark as not available but don't free yet completely
		// the rest is done in notifyOldFrameCompleted possibly a few frames later
		handle->nativeHandles.cpu.ptr = handle->nativeHandles.gpu.ptr = 0;
		handle->resident = false;

		handle->remove = true;
		staleQueue.emplace_back(queueNextIdx, handle);
	}

	ID3D12DescriptorHeap* const* DescriptorHeap::getPointerAddress()
	{
		BRWL_EXCEPTION(created, BRWL_CHAR_LITERAL("Invalid operation on unitialized heap."));
		return gpuHeap.GetAddressOf();
	}

	void DescriptorHeap::DescriptorRangeCopy(D3D12_CPU_DESCRIPTOR_HANDLE from, D3D12_CPU_DESCRIPTOR_HANDLE to, D3D12_CPU_DESCRIPTOR_HANDLE fromHeapStart, D3D12_CPU_DESCRIPTOR_HANDLE toHeapStart, unsigned int count)
	{
		ComPtr<ID3D12Device> device;
		gpuHeap->GetDevice(IID_PPV_ARGS(&device));

		if (fromHeapStart.ptr != toHeapStart.ptr)
		{
			device->CopyDescriptorsSimple(count, to, from, heapType);
			return;
		}

		// we are in the same heap and ranges may overlap
		if (from.ptr == to.ptr) return;

		// we only allow relocating towards the beginning of the heap
		BRWL_EXCEPTION(to.ptr <= from.ptr, nullptr);

		// copy first to last so that we do not copy into your own range
		unsigned int cpyStride = (from.ptr - to.ptr) / descriptorSize;
		unsigned int numCopied = 0;
		while (numCopied < count)
		{
			unsigned int n = (unsigned int) Utils::min((int64_t)cpyStride, (int64_t)count - (int64_t)numCopied);
			unsigned int step = n * descriptorSize;
			device->CopyDescriptorsSimple(n, to, from, heapType);
			from.ptr += step;
			to.ptr += step;
			numCopied += n;
		}

		//{	// copy last to first
		//	size_t cpyStride = (newOffset - oldOffset);
		//	size_t numCopied = 0;
		//	from.ptr += descriptorSize * handle->count;
		//	to.ptr += descriptorSize * (newOffset + handle->count);
		//	while (numCopied < handle->count)
		//	{
		//		unsigned int toCopy = (unsigned int)std::min((int64_t)cpyStride, (int64_t)handle->count - (int64_t)numCopied);
		//		size_t step = toCopy * descriptorSize;
		//		from.ptr -= step;
		//		to.ptr -= step;
		//		device->CopyDescriptorsSimple(toCopy, to, from, heapType);
		//		numCopied += toCopy;
		//	}

		//	handle->nativeHandles.cpu = to;
		//	handle->nativeHandles.gpu.ptr = gpuHeapGpuStart.ptr + handle->offset * descriptorSize;
		//}

	}

	void DescriptorHeap::maintain()
	{
		// move everything to the beginning of the heap on the cpu side
		std::scoped_lock(mutex);
		ComPtr<ID3D12Device> device;
		cpuHeap->GetDevice(IID_PPV_ARGS(&device));
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapCpuStart = cpuHeap->GetCPUDescriptorHandleForHeapStart();

		lastFree = 0;
		int j = 0;
		while (j < handles.size())
		{
			const int handleIdx = cpuOccupied[j];
			if (handleIdx != -1)
			{
				DescriptorHandle& handle = handles[handleIdx];
				BRWL_EXCEPTION(handle.count > 0, nullptr);
				for (int i = 0; i < handle.count; ++i)
				{
					BRWL_EXCEPTION(cpuOccupied[handle.offset + i] = handleIdx, nullptr);
					cpuOccupied[handle.offset + i] = -1;
					BRWL_EXCEPTION(cpuOccupied[lastFree + i] == -1, nullptr);
					cpuOccupied[lastFree + i] = handleIdx;
				}

				D3D12_CPU_DESCRIPTOR_HANDLE target = cpuHeapCpuStart;
				target.ptr += lastFree * descriptorSize;
				DescriptorRangeCopy(handle.nativeHandles.cpu, target, cpuHeapCpuStart, cpuHeapCpuStart, handle.count);

				handle.offset = lastFree;
				dirtyArray[handleIdx] = true;
				lastFree += handle.count;
				j += handle.count;
			}
			else {
				++j;
			}
		}

		if (lastFree) --lastFree;
	}

	void DescriptorHeap::notifyNewFrameStarted()
	{
		++queueNextIdx;
	}

	void DescriptorHeap::notifyOldFrameCompleted()
	{
		BRWL_CHECK(handles.size() == dirtyArray.size(), nullptr); // todo: remove
		++queueCompletedIdx;

		BRWL_EXCEPTION(queueCompletedIdx <= queueNextIdx, nullptr);

		std::scoped_lock(mutex);
		SCOPED_CPU_EVENT(0, 255, 0, "Descriptor Heap Maintain: %s", getHeapName(heapType, true));

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapCpuStart = cpuHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CPU_DESCRIPTOR_HANDLE gpuHeapCpuStart = gpuHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapGpuStart = gpuHeap->GetGPUDescriptorHandleForHeapStart();

		// first update layout on gpu heap
		for (int i = 0; i < dirtyArray.size(); ++i)
		{
	
			bool isDirty = dirtyArray[i];
			DescriptorHandle* handle = &handles[i];
			if (isDirty && handle->resident)
			{
				// handle is still marked resident, so we want to update its location on the gpu heap
				BRWL_EXCEPTION(handle->owningHeap != nullptr, BRWL_CHAR_LITERAL("Invalid descriptor state."));

				// update occupied flag for gpu tracking
				unsigned int oldGpuOffset = (handle->nativeHandles.residentCpu.ptr - gpuHeapCpuStart.ptr) / descriptorSize;
				for (int j = 0; j < handle->count; ++j)
				{
					BRWL_EXCEPTION(gpuOccupied[oldGpuOffset + j] == i, nullptr); // slot has to belong to us;
					gpuOccupied[oldGpuOffset + j] = -1;
					BRWL_EXCEPTION(gpuOccupied[handle->offset + j] == -1, nullptr); // slot has to be free;
					BRWL_EXCEPTION(cpuOccupied[handle->offset + j] == i, BRWL_CHAR_LITERAL("Inconsistent state of cpu descriptor tracker."));
					gpuOccupied[handle->offset + j] = i;
				}

				// we copy from the cpu heap and orphan the old gpu descriptor
				D3D12_CPU_DESCRIPTOR_HANDLE target = gpuHeapCpuStart;
				target.ptr += handle->offset * descriptorSize;
				DescriptorRangeCopy(handle->nativeHandles.cpu, target, cpuHeapCpuStart, gpuHeapCpuStart, handle->count);

				// update the resident descriptor handles
				handle->nativeHandles.residentCpu.ptr = gpuHeapCpuStart.ptr + handle->offset * descriptorSize;
				handle->nativeHandles.residentGpu.ptr = gpuHeapGpuStart.ptr + handle->offset * descriptorSize;

				dirtyArray[i] = false;
			}
		}

		// then copy new descriptors
		for (int i = 0; i < handles.size(); ++i)
		{
			bool isDirty = dirtyArray[i];
			DescriptorHandle* handle = &handles[i];
			if (isDirty && !handle->resident)
			{
				// if it's scheduled for removal then we do not copy the contents.
				if (handle->remove) continue;

				// update occupied flag for gpu tracking
				BRWL_EXCEPTION(cpuOccupied[handle->offset] == i, nullptr);
				BRWL_EXCEPTION(gpuOccupied[handle->offset] == -1, nullptr); // must be free
				gpuOccupied[handle->offset] = i;
				// we copy the initial contents from the cpu heap
				D3D12_CPU_DESCRIPTOR_HANDLE target = gpuHeapCpuStart;
				target.ptr += handle->offset * descriptorSize;
				DescriptorRangeCopy(handle->nativeHandles.cpu, target, cpuHeapCpuStart, gpuHeapCpuStart, handle->count);

				// update the resident descriptor handles
				handle->nativeHandles.residentCpu.ptr = gpuHeapCpuStart.ptr + handle->offset * descriptorSize;
				handle->nativeHandles.residentGpu.ptr = gpuHeapGpuStart.ptr + handle->offset * descriptorSize;

				handle->resident = true;
				dirtyArray[i] = false;
			}
		}

		// then delete old decriptors
		while (!staleQueue.empty() && std::get<0>(staleQueue.front()) <= queueCompletedIdx)
		{
			DescriptorHandle* handle = std::get<1>(staleQueue.front());
			staleQueue.pop_front();
			BRWL_EXCEPTION(handle->remove, nullptr);
			BRWL_EXCEPTION(gpuOccupied[handle->offset] != -1, nullptr);
			BRWL_EXCEPTION(cpuOccupied[handle->offset] != -1, nullptr);
			// mark as actually free now
			numOccupiedDescriptors -= handle->count;
			for (unsigned int j = 0; j < handle->count; ++j)
			{
				BRWL_EXCEPTION(gpuOccupied[handle->offset + j] == handle - handles.data(), nullptr);
				BRWL_EXCEPTION(cpuOccupied[handle->offset + j] == handle - handles.data(), nullptr);
				gpuOccupied[handle->offset + j] = -1;
				cpuOccupied[handle->offset + j] = -1;
			}

			handle->nativeHandles.residentCpu.ptr = handle->nativeHandles.residentCpu.ptr = 0;
			handle->name = nullptr;
			handle->owningHeap = nullptr;
			handle->offset = 0;
			handle->count = 0;
			handle->remove = false;
			handle->resident = false;
		}
	}
	
}

BRWL_RENDERER_NS_END