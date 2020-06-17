#pragma once // (c) 2020 Lukas Brunner

#include <vector>
#include <array>
#include <mutex>

BRWL_RENDERER_NS

namespace PAL
{
    class DescriptorHeap;

    class DescriptorHandle final
    {
        friend class DescriptorHeap;
    public:
        struct NativeHandles
        {
            D3D12_CPU_DESCRIPTOR_HANDLE residentCpu; // gpu visible descriptor - used for rendering
            D3D12_GPU_DESCRIPTOR_HANDLE residentGpu; // gpu visible descriptor - used for rendering
            D3D12_CPU_DESCRIPTOR_HANDLE cpu; // cpu side descriptor - used for view creation - later copied to the gpu visible heap
            D3D12_GPU_DESCRIPTOR_HANDLE gpu; // should not be accessed
        };
    private:
        // No touchy thouchy
        DescriptorHandle(DescriptorHandle const&) = delete;
        DescriptorHandle& operator=(DescriptorHandle const&) = delete;
        NativeHandles nativeHandles;
        DescriptorHeap* owningHeap;
        unsigned int offset;
        unsigned int count;
        const BRWL_CHAR* name;
        bool resident;
    public:
        // Needed for vector, but do not use!
        DescriptorHandle();
        DescriptorHandle(DescriptorHandle&&) = default;
        DescriptorHandle& operator = (DescriptorHandle&&) = default;
        NativeHandles operator[](int idx);
        bool isResident() { return resident; }
        void release();

        const D3D12_CPU_DESCRIPTOR_HANDLE& getCpu() { BRWL_EXCEPTION(resident && count == 1, nullptr); return nativeHandles.cpu; }
        const D3D12_GPU_DESCRIPTOR_HANDLE& getGpu() { BRWL_EXCEPTION(resident && count == 1, nullptr); return nativeHandles.gpu; }

    };

    // Reorganizing descriptor heap for allocating single handles and ranges
    class DescriptorHeap final
    {
        friend class DescriptorHandle;
    public:

        static const int maxFramesInFlight = 3;

        DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
        ~DescriptorHeap();

        bool create(ID3D12Device* device, unsigned int numDescriptors);
        void destroy();
        // Allocates a single handle. The result is immediate and the handle is resident / can be used for resource creation
        DescriptorHandle* allocateOne(const BRWL_CHAR* name);
        void releaseOne(DescriptorHandle* handle);
        // Allocates a consecutive range of handles and returns a pointer to the first handle.
        // The result is not immediate as the heap may have to move descriptors around to make space.
        // Check the isResident flag on the returned handle to see when the handle can be used.
        DescriptorHandle* allocateRange(unsigned int n, const BRWL_CHAR* name);
        void releaseRange(DescriptorHandle* handle);

        void update();
        // after maintain lastFree points to the last used descriptor index
        void maintain();

        ID3D12DescriptorHeap* const* getPointerAddress();
        D3D12_DESCRIPTOR_HEAP_TYPE getType() const { return heapType; }
        bool isCreated() const { return created; }

        void notifyNewFrameStarted();
        void notifyOldFrameCompleted();
    private:
        std::recursive_mutex mutex;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType;

        unsigned int descriptorSize;
        unsigned int numOccupiedDescriptors;
        ComPtr<ID3D12DescriptorHeap> cpuHeap;
        ComPtr<ID3D12DescriptorHeap> gpuHeap;
        int queueCompletedIdx;
        int queueNextIdx;
        // indices in the dirty queue are relative to the cpu heap
        std::vector<std::array<std::tuple<unsigned int, bool>, maxFramesInFlight>> dirtyQueue;
        // Handles stay in place
        std::vector<DescriptorHandle> handles;
        // indices move upon maintenance
        std::vector<int> cpuOccupied; // allways up to date
        std::vector<int> gpuOccupied;
        int lastHandle; // relative to cpu heap
        int lastFree; // relative to cpu heap
        bool created;
    };


}

BRWL_RENDERER_NS_END