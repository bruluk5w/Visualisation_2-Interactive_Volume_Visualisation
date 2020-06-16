#pragma once // (c) 2020 Lukas Brunner

#include <vector>
#include <array>
#include <mutex>

BRWL_RENDERER_NS

namespace PAL
{

    // Reorganizing descriptor heap for allocating single handles and ranges
    class DescriptorHeap final
    {
    public:

        static const int maxFramesInFlight = 3;

        class Handle
        {
            friend class DescriptorHeap;

            // No touchy thouchy
            Handle(Handle const&) = delete;
            Handle& operator=(Handle const&) = delete;

            D3D12_CPU_DESCRIPTOR_HANDLE cpu;
            D3D12_GPU_DESCRIPTOR_HANDLE gpu;
            DescriptorHeap* owningHeap;
            size_t offset;
            bool isRange;
            bool isFirst;
            const BRWL_CHAR* name;
            bool isResident;
        public:
            // Needed for vector, but do not use!
            Handle();
            Handle(Handle&&) = default;
            Handle& operator = (Handle&&) = default;

            void release();

            const D3D12_CPU_DESCRIPTOR_HANDLE& getCpu() { BRWL_EXCEPTION(isResident, nullptr); return cpu; }
            const D3D12_GPU_DESCRIPTOR_HANDLE& getGpu() { BRWL_EXCEPTION(isResident, nullptr); return gpu; }

        };

        DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
        ~DescriptorHeap();

        bool create(ID3D12Device* device, unsigned int numDescriptors);
        void destroy();
        // Allocates a single handle. The result is immediate and the handle is resident / can be used for resource creation
        Handle* allocateOne(const BRWL_CHAR* name);
        void releaseOne(Handle* handle);
        // Allocates a consecutive range of handles and returns a pointer to the first handle.
        // The result is not immediate as the heap may have to move descriptors around to make space.
        // Check the isResident flag on the returned handle to see when the handle can be used.
        Handle* allocateRange(unsigned int n, const BRWL_CHAR* name);
        //Handle* releaseRange(Handle* handle);

        void update();
        //void maintain();

        ID3D12DescriptorHeap* const* getPointerAddress();
        D3D12_DESCRIPTOR_HEAP_TYPE getType() const { return heapType; }
        bool isCreated() const { return created; }

        void notifyNewFrameStarted();
        void notifyOldFrameCompleted();
    private:
        std::mutex mutex;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType;

        size_t descriptorSize;
        unsigned int numOccupiedDescriptors;
        ComPtr<ID3D12DescriptorHeap> cpuHeap;
        ComPtr<ID3D12DescriptorHeap> gpuHeap;
        unsigned int queueCompletedIdx;
        unsigned int queueNextIdx;
        std::vector<std::array<std::tuple<unsigned int, bool>, maxFramesInFlight>> dirtyQueue;
        std::vector<Handle> handles;
        size_t nextOffset;
        bool created;
    };


}

BRWL_RENDERER_NS_END