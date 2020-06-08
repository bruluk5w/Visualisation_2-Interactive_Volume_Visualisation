#pragma once // (c) 2020 Lukas Brunner

#include <vector>

BRWL_RENDERER_NS

namespace PAL
{


    class DescriptorHeap final
    {
    public:
        struct Handle
        {
            Handle(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu, DescriptorHeap* heap, size_t offset
#ifdef _DEBUG
                , BRWL_STR name
#endif
            ) :
                cpu(cpu), gpu(gpu), owningHeap(heap), offset(offset)
#ifdef _DEBUG
                , name(std::move(name))
#endif
            { }

            Handle() : cpu{ 0 }, gpu{ 0 }, owningHeap(nullptr), offset(0)
            { }

            void destroy() {
                if (owningHeap) {
                    owningHeap->returnHandle(*this);
                }
            }

            D3D12_CPU_DESCRIPTOR_HANDLE cpu;
            D3D12_GPU_DESCRIPTOR_HANDLE gpu;
            DescriptorHeap* owningHeap;
            size_t offset;
#ifdef _DEBUG
            BRWL_STR name;
#endif
        };

        DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS shaderVisibleFlag);
        ~DescriptorHeap();

        bool create(ID3D12Device* device, unsigned int numDescriptors);
        void destroy();
        Handle allocateHandle(
#ifdef _DEBUG
            BRWL_STR name
#endif
        );
        void returnHandle(Handle& handle);
        ID3D12DescriptorHeap* const* getPointerAddress();
        D3D12_DESCRIPTOR_HEAP_TYPE getType() const { return heapType; }
        bool isShaderVisible() const { return shaderVisibleFlag == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; }
        bool isCreated() const { return created; }

    private:
        bool created;
        bool full;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType;
        D3D12_DESCRIPTOR_HEAP_FLAGS shaderVisibleFlag;
        ComPtr<ID3D12DescriptorHeap> heap;
        size_t descriptorSize;
        unsigned int numDescriptors;
        std::unique_ptr<uint8_t[]> occupied;
        unsigned int occupiedNumAllocatedBytes;
        size_t nextOffset;
#ifdef _DEBUG
        struct TrackingEntry {
            BRWL_STR name;
            size_t offset;
        };
        std::vector<TrackingEntry> trackingList;
#endif
    };


}

BRWL_RENDERER_NS_END