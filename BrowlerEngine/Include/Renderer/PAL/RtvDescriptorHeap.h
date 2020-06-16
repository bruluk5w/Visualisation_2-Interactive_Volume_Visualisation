#pragma once // (c) 2020 Lukas Brunner

#include <vector>

BRWL_RENDERER_NS

namespace PAL
{

    // As opposed to the DescriptorHeap, the RtvDescriptor heap diretly alloctates its descriptors in a gpu visible heap
    // Thus no internal reorganization is possible and only single handles can be obtained
    class RtvDescriptorHeap final
    {
    public:
        struct Handle
        {
            Handle(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu, RtvDescriptorHeap* heap, size_t offset
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
            RtvDescriptorHeap* owningHeap;
            size_t offset;
#ifdef _DEBUG
            BRWL_STR name;
#endif
        };

        RtvDescriptorHeap();
        ~RtvDescriptorHeap();

        bool create(ID3D12Device* device, unsigned int numDescriptors);
        void destroy();
        Handle allocateHandle(
#ifdef _DEBUG
            BRWL_STR name
#endif
        );
        void returnHandle(Handle& handle);
        ID3D12DescriptorHeap* const* getPointerAddress();
        bool isCreated() const { return created; }

    private:
        bool created;
        bool full;
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
            size_t frameIdx;
        };
    public:
        void notifyNewFrame() { ++frameIdx; }
    private:
        size_t frameIdx;
        std::vector<TrackingEntry> home;
        std::vector<TrackingEntry> away;
#endif
    };


}

BRWL_RENDERER_NS_END