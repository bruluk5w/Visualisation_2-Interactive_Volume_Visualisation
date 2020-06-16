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

	DescriptorHeap::Handle::Handle() :
		cpu{ 0 },
		gpu{ 0 },
		owningHeap(nullptr),
		offset(0),
		isRange(false),
		isFirst(false),
		name(nullptr),
		isResident(false)
	{ }

	void DescriptorHeap::Handle::release()
	{
		if (!owningHeap) return;
		if (!isRange)
		{
			owningHeap->releaseOne(this);
		}
		else
		{
			BRWL_EXCEPTION(false, nullptr);
			//owningHeap->releaseRange(this);
		}

		BRWL_EXCEPTION(false, BRWL_CHAR_LITERAL("Invalid descriptor handle release."));
	}


	DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) :
		heapType(type),
		descriptorSize(0),
		numOccupiedDescriptors(0),
		cpuHeap(nullptr),
		gpuHeap(nullptr),
		queueCompletedIdx(0),
		queueNextIdx(0),
		dirtyQueue(),
		nextOffset(0),
		handles(),
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
		dirtyQueue.resize(capacity);

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
		dirtyQueue.clear();
		handles.clear();
		nextOffset = 0;
	}

	DescriptorHeap::Handle* DescriptorHeap::allocateOne(const BRWL_CHAR* name)
	{
		std::scoped_lock(mutex);

		BRWL_EXCEPTION(numOccupiedDescriptors < handles.size(), BRWL_CHAR_LITERAL("Cannot allocate another descriptor handle. Heap full."));

		Handle* handle = &handles[nextOffset];

		BRWL_CHECK(handle->owningHeap == nullptr, nullptr);
		
		D3D12_CPU_DESCRIPTOR_HANDLE cpu = gpuHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu = gpuHeap->GetGPUDescriptorHandleForHeapStart();

		handle->owningHeap = this;
		handle->isRange = false;
		handle->offset = nextOffset;
		handle->name = name;
		handle->cpu.ptr = cpu.ptr + descriptorSize * handle->offset;
		handle->gpu.ptr = gpu.ptr + descriptorSize * handle->offset;
		handle->isResident = true;

		++numOccupiedDescriptors;

		if (numOccupiedDescriptors < handles.size())
		{
			for (int i = 0; i < handles.size(); ++i)
			{
				nextOffset = (nextOffset + 1) % handles.size();
				if (handles[nextOffset].owningHeap == nullptr)
					break;
			}
		}

		return handle;
	}

	void DescriptorHeap::releaseOne(Handle* handle)
	{
		std::scoped_lock(mutex);

		BRWL_EXCEPTION(handle && handle->owningHeap == this, nullptr);
		size_t idx = handle - handles.data();
		BRWL_EXCEPTION(idx > 0 && idx < handles.size(), nullptr);
		BRWL_EXCEPTION(!handle->isRange, nullptr);

		// mark as not available but don't free yet completely
		// the rest is done in notifyOldFrameCompleted possibly a few frames later
		handle->cpu.ptr = handle->gpu.ptr = 0;
		handle->isRange = handle->isFirst = false;
		handle->isResident = false;
		std::get<bool>(dirtyQueue[idx][queueNextIdx % maxFramesInFlight]) = true;
		std::get<unsigned int>(dirtyQueue[idx][queueNextIdx % maxFramesInFlight]) = queueNextIdx;
	}

	//DescriptorHeap::Handle* DescriptorHeap::allocateRange(unsigned int n, const BRWL_CHAR* name)
	//{
	//	std::scoped_lock(mutex);

	//	BRWL_EXCEPTION(numOccupiedDescriptors  + n <= handles.size(), BRWL_CHAR_LITERAL("Cannot allocate requested descriptor range. Heap too full."));

	//	D3D12_CPU_DESCRIPTOR_HANDLE cpu = cpuHeap->GetCPUDescriptorHandleForHeapStart();
	//	D3D12_GPU_DESCRIPTOR_HANDLE gpu = gpuHeap->GetGPUDescriptorHandleForHeapStart();
	//	size_t handleOffset = nextOffset;

	//	Handle* handle = &handles[nextOffset];

	//	BRWL_CHECK(handle->owningHeap == nullptr, nullptr);

	//	handle->owningHeap = this;
	//	handle->cpu.ptr = cpu.ptr + nextOffset * descriptorSize;
	//	handle->gpu.ptr = gpu.ptr + nextOffset * descriptorSize;
	//	handle->isRange = false;
	//	handle->offset = nextOffset;
	//	handle->name = name;

	//	++numOccupiedDescriptors;

	//	if (numOccupiedDescriptors < handles.size())
	//	{
	//		for (int i = 0; i < handles.size(); ++i)
	//		{
	//			nextOffset = (nextOffset + 1) % handles.size();
	//			if (handles[nextOffset].owningHeap == nullptr)
	//				break;
	//		}
	//	}

	//	return handle;
	//}


	//void DescriptorHeap::update()
	//{
	//	BRWL_EXCEPTION(queueCompletedIdx < queueNextIdx, nullptr);
	//	BRWL_EXCEPTION(queueNextIdx - queueCompletedIdx < maxFramesInFlight, nullptr);
	//	std::scoped_lock(mutex);
	//	SCOPED_CPU_EVENT(255, 0, 0, "Descriptor Heap Update %s", getHeapName(heapType, true));
	//	ComPtr<ID3D12Device> device;
	//	cpuHeap->GetDevice(IID_PPV_ARGS(&device));
	//	BRWL_CHECK(handles.size() == dirtyQueue.size(), nullptr);

	//	D3D12_CPU_DESCRIPTOR_HANDLE cpuHeapCpuStart = cpuHeap->GetCPUDescriptorHandleForHeapStart();

	//	D3D12_CPU_DESCRIPTOR_HANDLE gpuHeapCpuStart = cpuHeap->GetCPUDescriptorHandleForHeapStart();
	//	D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapGpuStart = gpuHeap->GetGPUDescriptorHandleForHeapStart();

	//	D3D12_CPU_DESCRIPTOR_HANDLE tmp;
	//	for (int i = 0; i < handles.size(); ++i)
	//	{
	//		unsigned int completed = queueCompletedIdx % maxFramesInFlight;
	//		std::tuple<int, bool> (&queue)[maxFramesInFlight] = dirtyQueue[i];
	//		if (queue)
	//		{
	//			Handle* handle = &handles[i];
	//			if (handle->owningHeap != nullptr)
	//			{
	//				size_t offset = descriptorSize * handle->offset;
	//				tmp.ptr = cpuHeapCpuStart.ptr + offset;
	//				handle->cpu.ptr = gpuHeapCpuStart.ptr + offset;
	//				handle->gpu.ptr = gpuHeapGpuStart.ptr + offset,
	//				device->CopyDescriptorsSimple(1, handle->cpu, tmp, heapType);
	//				handle->isResident = true;
	//				dirty[i] = false;
	//			}
	//		}
	//	}

	//	device->CopyDescriptors()
	//}

	ID3D12DescriptorHeap* const* DescriptorHeap::getPointerAddress()
	{
		BRWL_EXCEPTION(created, BRWL_CHAR_LITERAL("Invalid operation on unitialized heap."));
		return gpuHeap.GetAddressOf();
	}

	void DescriptorHeap::notifyNewFrameStarted()
	{
		++queueNextIdx;
	}

	void DescriptorHeap::notifyOldFrameCompleted()
	{
		BRWL_CHECK(handles.size() == dirtyQueue.size(), nullptr); // todo: remove
		++queueCompletedIdx;

		BRWL_EXCEPTION(queueCompletedIdx < queueNextIdx, nullptr);
		BRWL_EXCEPTION(queueNextIdx - queueCompletedIdx < maxFramesInFlight, nullptr);
		std::scoped_lock(mutex);
		SCOPED_CPU_EVENT(0, 255, 0, "Descriptor Heap Maintain: %s", getHeapName(heapType, true));

		ComPtr<ID3D12Device> device;
		gpuHeap->GetDevice(IID_PPV_ARGS(&device));

		D3D12_CPU_DESCRIPTOR_HANDLE gpuHeapCpuStart = gpuHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapGpuStart = gpuHeap->GetGPUDescriptorHandleForHeapStart();

		for (int i = 0; i < handles.size(); ++i)
		{
			unsigned int completed = queueCompletedIdx % maxFramesInFlight;
			std::tuple<unsigned int, bool>& frameSlot = dirtyQueue[i][completed];
			bool& isDirty = std::get<bool>(frameSlot);
			if (isDirty)
			{
				unsigned int& frameIdx = std::get<unsigned int>(frameSlot);
				BRWL_EXCEPTION(frameIdx == queueCompletedIdx, BRWL_CHAR_LITERAL("Inconsistent dirty queue state."));
				Handle* handle = &handles[i];
				BRWL_EXCEPTION(handle->owningHeap != nullptr, BRWL_CHAR_LITERAL("Invalid descriptor state."));
				if (!handle->isResident)
				{
					// mark as actually free now
					handle->name = nullptr;
					handle->owningHeap = nullptr;
					handle->offset = 0;
				}
				else
				{	
					// handle is still marked resident, so we probably want to update its location on the gpu heap
					const size_t oldOffset = (handle->cpu.ptr - gpuHeapCpuStart.ptr) / descriptorSize;
					BRWL_EXCEPTION(oldOffset != handle->offset, nullptr);
					const size_t newOffset = handle->offset;
					if (handle->isRange)
					{
						BRWL_EXCEPTION(handle->isFirst, nullptr);
						// TODO: use range copy if not overlapping
						if (newOffset < oldOffset)
						{	// copy first to last so that we do not copy into your own range
							BRWL_EXCEPTION(false, nullptr);
						}
						else
						{	// copy last to first
							BRWL_EXCEPTION(false, nullptr);
						}
					}
					else
					{
						D3D12_CPU_DESCRIPTOR_HANDLE tmp = gpuHeapCpuStart;
						tmp.ptr += descriptorSize * newOffset;
						device->CopyDescriptorsSimple(1, tmp, handle->cpu, heapType);
						handle->cpu = tmp;
						handle->gpu.ptr = gpuHeapGpuStart.ptr + descriptorSize * newOffset;
					}
				}
				// rest dirtyflag
				frameIdx = 0;
				isDirty = false;
			}
		}
	}

}

BRWL_RENDERER_NS_END