#pragma once // (c) 2020 Lukas Brunner

#include <vector>
#include <mutex>
#include <deque>

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
        unsigned int offset;  // merely convenience, could also be calculated from heap start  and cpuHeap cpu ptr
        unsigned int count;
        const BRWL_CHAR* name;
        bool resident;
        bool remove;
    public:
        // Needed for vector, but do not use!
        DescriptorHandle();
        DescriptorHandle(DescriptorHandle&&) = default;
        DescriptorHandle& operator = (DescriptorHandle&&) = default;
        NativeHandles getResident(int idx);
        NativeHandles getNonResident(int idx);
        bool isResident() { return resident && !remove; }
        void release();

        D3D12_CPU_DESCRIPTOR_HANDLE getCpu();
        D3D12_GPU_DESCRIPTOR_HANDLE getGpu() { BRWL_EXCEPTION(resident && count == 1, nullptr); return nativeHandles.residentGpu; }

    };

    // Reorganizing descriptor heap for allocating single handles and ranges
    class DescriptorHeap final
    {
        friend class DescriptorHandle;
    public:

        DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
        ~DescriptorHeap();

        bool create(ID3D12Device* device, unsigned int numDescriptors);
        void destroy();
        // Allocates a single handle. Only use force update when you know that the gpu will not access any resources at all
        DescriptorHandle* allocateOne(const BRWL_CHAR* name, bool forceUpdate=false);
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
        void DescriptorRangeCopy(D3D12_CPU_DESCRIPTOR_HANDLE from, D3D12_CPU_DESCRIPTOR_HANDLE to, D3D12_CPU_DESCRIPTOR_HANDLE fromHeapStart, D3D12_CPU_DESCRIPTOR_HANDLE toHeapStart, unsigned int count);

        std::recursive_mutex mutex;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType;

        unsigned int descriptorSize;
        unsigned int numOccupiedDescriptors;
        ComPtr<ID3D12DescriptorHeap> cpuHeap;
        ComPtr<ID3D12DescriptorHeap> gpuHeap;
        int queueCompletedIdx;
        int queueNextIdx;
        // items that are about to be delted but may still be referenced by the gpu
        std::deque<std::tuple<unsigned int, DescriptorHandle*>> staleQueue;
        // true at index n if n-th DescriptorHandle in "handles" is dirty
        std::vector<bool> dirtyArray;
        // Handles stay in place
        std::vector<DescriptorHandle> handles;
        // indices move upon maintenance
        // these 2 vectors are indices of the DescriptorHandle instances which own the respective descriptor
        std::vector<int> cpuOccupied; // always up to date
        std::vector<int> gpuOccupied; // continuously updated when frames finish
        int lastHandle; // relative to cpu heap
        int lastFree; // relative to cpu heap
        bool created;
    };


}

BRWL_RENDERER_NS_END