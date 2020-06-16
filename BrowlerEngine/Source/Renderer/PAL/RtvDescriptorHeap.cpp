#include "PAL/RtvDescriptorHeap.h"

BRWL_RENDERER_NS

namespace PAL
{


	RtvDescriptorHeap::RtvDescriptorHeap() :
		created(false),
		full(false),
		heap(nullptr),
		descriptorSize(0),
		numDescriptors(0),
		occupied(),
		occupiedNumAllocatedBytes(0),
		nextOffset(0)
#ifdef _DEBUG
		, home()
		, away()
		, frameIdx(0)
#endif
	{ }

	RtvDescriptorHeap::~RtvDescriptorHeap()
	{
		destroy();
	}

	bool RtvDescriptorHeap::create(ID3D12Device* device, unsigned int minNumDescriptors)
	{
		destroy();

		D3D12_DESCRIPTOR_HEAP_DESC desc = { };
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		const unsigned int occupiedArrayByteLen = minNumDescriptors / 8 + 1;
		numDescriptors = 8 * occupiedArrayByteLen;
		desc.NumDescriptors = numDescriptors;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		if (BRWL_VERIFY(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))), BRWL_CHAR_LITERAL("Failed to create render target view descriptor heap.")))
		{
			descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			if (occupied == nullptr || occupiedNumAllocatedBytes < occupiedArrayByteLen)
			{
				occupied = std::unique_ptr<uint8_t[]>(new uint8_t[occupiedArrayByteLen]);
				occupiedNumAllocatedBytes = occupiedArrayByteLen;
				memset(occupied.get(), 0, numDescriptors / 8);
			}

			heap->SetName(BRWL_CHAR_LITERAL("RTV HEAP"));
			created = true;
		}
		else
		{
			heap = nullptr;
		}

		return created;
	}

	void RtvDescriptorHeap::destroy()
	{
#ifndef SUBMISSION
		for (unsigned int i = 0; i < occupiedNumAllocatedBytes; ++i) {
			BRWL_EXCEPTION(occupied[i] == 0, BRWL_CHAR_LITERAL("Descriptor heap destroyed while descriptors are still away on business."));
		}
#endif
		created = false;
		full = false;
		heap = nullptr;
		nextOffset = 0;
		numDescriptors = 0;
		descriptorSize = 0;
		// no need to allocate this again next time since this will mostly be destroyed when resizing the frame buffers
		if (occupied != nullptr)
			memset(occupied.get(), 0, occupiedNumAllocatedBytes);
	}

	RtvDescriptorHeap::Handle RtvDescriptorHeap::allocateHandle(
#ifdef _DEBUG
		BRWL_STR name
#endif
	)
	{
		BRWL_EXCEPTION(!full, BRWL_CHAR_LITERAL("Cannot allocate another descriptor handle. Heap full."));

		D3D12_CPU_DESCRIPTOR_HANDLE cpu = heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu = heap->GetGPUDescriptorHandleForHeapStart();
		size_t handleOffset = nextOffset;

		{
			const size_t byteIdx = nextOffset / 8;
			const size_t bitIdx = nextOffset - 8 * byteIdx;
			occupied[byteIdx] |= 1 << bitIdx;
		}

		cpu.ptr += (nextOffset)*descriptorSize;
		gpu.ptr += (nextOffset)*descriptorSize;

		bool foundFree = false;
		for (unsigned int i = 0; i < numDescriptors; ++i) {
			nextOffset = (nextOffset + 1) % numDescriptors;

			const size_t byteIdx = nextOffset / 8;
			const size_t bitIdx = nextOffset - 8 * byteIdx;
			if (((occupied[byteIdx] >> bitIdx) & 1) == 0)
			{
				foundFree = true;
				break;
			}
		}

		if (!foundFree) full = true;

#ifdef _DEBUG
		auto it = std::find_if(home.begin(), home.end(), [handleOffset](TrackingEntry& e) {return e.offset == handleOffset; });
		if (it != home.end()) {
			home.erase(it);
		}
		away.push_back({ name, handleOffset, frameIdx });
#endif

		return { cpu, gpu , this, handleOffset,
#ifdef _DEBUG
			std::move(name)
#endif
		};
	}

	void RtvDescriptorHeap::returnHandle(Handle& handle)
	{
#ifdef _DEBUG
		auto it = std::find_if(away.begin(), away.end(), [&handle](TrackingEntry& e) {return e.offset == handle.offset; });
		if (it != away.end()) {
			away.erase(it);
		}
		home.push_back({ handle.name, handle.offset, frameIdx });
#endif

		const size_t byteIdx = handle.offset / 8;
		const size_t bitIdx = handle.offset - 8 * byteIdx;
		occupied[byteIdx] &= ~(1 << bitIdx);
		handle.cpu.ptr = handle.gpu.ptr = 0;
		handle.owningHeap = nullptr;
		handle.offset = 0;
	}

	ID3D12DescriptorHeap* const* RtvDescriptorHeap::getPointerAddress()
	{
		BRWL_EXCEPTION(created, BRWL_CHAR_LITERAL("Invalid operation on unitialized heap."));
		return heap.GetAddressOf();
	}


}

BRWL_RENDERER_NS_END