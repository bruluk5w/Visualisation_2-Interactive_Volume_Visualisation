#include "PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

namespace PAL
{


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
	}

	DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS shaderVisibleFlag) : 
		created(false),
		full(false),
		heapType(type),
		shaderVisibleFlag(shaderVisibleFlag),
		heap(nullptr),
		descriptorSize(0),
		numDescriptors(0),
		occupied(),
		occupiedNumAllocatedBytes(0),
		nextOffset(0)
	{ }

	bool DescriptorHeap::create(ID3D12Device* device, unsigned int minNumDescriptors)
	{
		destroy();

		D3D12_DESCRIPTOR_HEAP_DESC desc = { };
		desc.Type = heapType;
		const unsigned int occupiedArrayByteLen = minNumDescriptors / 8 + 1;
		numDescriptors = 8 * occupiedArrayByteLen;
        desc.NumDescriptors = numDescriptors;
        desc.Flags = shaderVisibleFlag;
        desc.NodeMask = 0;

		if (BRWL_VERIFY(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap))), getErrorMessge(heapType)))
		{
			descriptorSize = device->GetDescriptorHandleIncrementSize(heapType);
			if (occupied == nullptr || occupiedNumAllocatedBytes < occupiedArrayByteLen)
			{
				occupied = std::unique_ptr<uint8_t[]>(new uint8_t[occupiedArrayByteLen]);
				occupiedNumAllocatedBytes = occupiedArrayByteLen;
				memset(occupied.get(), 0, numDescriptors / 8);
			}

			created = true;
		}
		else
		{
			heap = nullptr;
        }

        return created;
	}

	void DescriptorHeap::destroy()
	{
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

	DescriptorHeap::Handle DescriptorHeap::allocateHandle()
	{
		BRWL_EXCEPTION(!full, BRWL_CHAR_LITERAL("Cannot allocate another descriptor handle. Heap full."));

		D3D12_CPU_DESCRIPTOR_HANDLE cpu = heap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpu = heap->GetGPUDescriptorHandleForHeapStart();

		{
			const size_t byteIdx = nextOffset / 8;
			const size_t bitIdx = nextOffset - 8 * byteIdx;
			occupied[byteIdx] |= 1 << bitIdx;
		}

		cpu.ptr += (nextOffset) * descriptorSize;
		gpu.ptr += (nextOffset) * descriptorSize;

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

		return { cpu, gpu };
	}

	ID3D12DescriptorHeap* const* DescriptorHeap::getPointerAddress()
	{
		BRWL_EXCEPTION(created, BRWL_CHAR_LITERAL("Invalid operation on unitialized heap."));
		return heap.GetAddressOf();
	}


}

BRWL_RENDERER_NS_END