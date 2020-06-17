#include "ComputeBuffers.h"
#include "Core/BrowlerEngine.h"
#include "Common/Logger.h"

BRWL_RENDERER_NS


ComputeBuffers::ComputeBuffers() :
    bufferWidth(0),
    bufferHeight(0),
    buffers{ nullptr },
    bufferHeap(nullptr),
    srvDescriptorRange(nullptr),
    uavDescriptorRange(nullptr),
    created(false),
    pingPong(false)
{ }

bool ComputeBuffers::create(ID3D12Device* device, PAL::DescriptorHeap* srvHeap, unsigned int width, unsigned int height)
{
    BRWL_CHECK(srvHeap && srvHeap->getType() == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && srvHeap->isCreated(), nullptr);

    bufferWidth = width;
    bufferHeight = height;

    D3D12_RESOURCE_DESC textureDecriptions[countof(*(decltype(buffers)*)(0))];
    memset(&textureDecriptions, 0, sizeof(textureDecriptions));
    textureDecriptions[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDecriptions[0].Alignment = 0; // should choose D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT automatically
    textureDecriptions[0].Width = bufferWidth;
    textureDecriptions[0].Height = bufferHeight;
    textureDecriptions[0].DepthOrArraySize = 1;
    textureDecriptions[0].MipLevels = 1;
    textureDecriptions[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    textureDecriptions[0].SampleDesc = { 1, 0 };
    textureDecriptions[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDecriptions[0].Flags = D3D12_RESOURCE_FLAG_NONE;

    // for now we use the same format for all buffers 
    for (int i = 1; i < 6; ++i) {
        textureDecriptions[i << 1] = textureDecriptions[0];
    }
    // copy to 2nd layer
    for (int i = 0; i < 6; ++i) {
        textureDecriptions[i << 1 | 1] = textureDecriptions[i << 1];
    }

    // how much memory should we allocate?
    ComPtr<ID3D12Device4> device4;
    if (!BRWL_VERIFY(SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&device4))), nullptr))
    {
        destroy();
        return false;
    }
    D3D12_RESOURCE_ALLOCATION_INFO1 allocInfoEx[countof(textureDecriptions)];
    D3D12_RESOURCE_ALLOCATION_INFO allocInfo = device4->GetResourceAllocationInfo1(0, countof(textureDecriptions), textureDecriptions, allocInfoEx);
    {
        BRWL_CHAR buf[100];
        BRWL_SNPRINTF(buf, countof(buf), BRWL_CHAR_LITERAL("Allocating compute buffers %zu x %u x %u. Required VRAM: %.2fMB"),
            countof(textureDecriptions), bufferWidth, bufferHeight, (float)allocInfo.SizeInBytes / 1048576.f);
        engine->logger->info(buf);
    }

    // allocate heap
    D3D12_HEAP_DESC heapDesc;
    memset(&heapDesc, 0, sizeof(heapDesc));
    heapDesc.SizeInBytes = allocInfo.SizeInBytes;
    heapDesc.Alignment = allocInfo.Alignment;
    heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT; // some gpu only memory
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;

    // Dear GPU,
    // Please take it or die!
    if (!BRWL_VERIFY(SUCCEEDED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&bufferHeap))), BRWL_CHAR_LITERAL("Failed to allocate compute buffers. It's not a bug. It's your VRAM.")))
    {
        destroy();
        return false;
    }


    //Place resources
    for (int i = 0; i < countof(allocInfoEx); ++i)
    {
        D3D12_RESOURCE_ALLOCATION_INFO1& info = allocInfoEx[i];
        if (!BRWL_VERIFY(SUCCEEDED(device->CreatePlacedResource(bufferHeap.Get(), info.Offset, &textureDecriptions[i], D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffers[i]))), BRWL_CHAR_LITERAL("Failed to place resource into heap.")))
        {
            destroy();
            return false;
        }
    }

    // Create UAVs && SRVs for the textures
    srvDescriptorRange = srvHeap->allocateRange(countof(buffers), BRWL_CHAR_LITERAL("SRV COMPUTE BUFFERS"));
    uavDescriptorRange = srvHeap->allocateRange(countof(buffers), BRWL_CHAR_LITERAL("UAV COMPUTE BUFFERS"));
    for (int i = 0; i < countof(allocInfoEx); ++i)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        memset(&srvDesc, 0, sizeof(srvDesc));
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDecriptions[i].Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        device->CreateShaderResourceView(buffers[i].Get(), &srvDesc, (*srvDescriptorRange)[i].cpu);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        memset(&uavDesc, 0, sizeof(uavDesc));
        uavDesc.Format = textureDecriptions[i].Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
        device->CreateUnorderedAccessView(buffers[i].Get(), nullptr, &uavDesc, (*uavDescriptorRange)[i].cpu);
    }

    created = true;
    return created;
}

void ComputeBuffers::destroy()
{
    created = false;

    bufferWidth = 0;
    bufferHeight = 0;
    for (int i = 0; i < countof(*(decltype(buffers)*)(0)); ++i)
    {
        buffers[i] = nullptr;
    }

    bufferHeap = nullptr;
    srvDescriptorRange = nullptr;
    uavDescriptorRange = nullptr;
}

bool ComputeBuffers::isResident()
{
    return srvDescriptorRange && srvDescriptorRange->isResident() && uavDescriptorRange && uavDescriptorRange->isResident();
}

PAL::DescriptorHandle::NativeHandles ComputeBuffers::getSourceSrv(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return (*srvDescriptorRange)[idx + pingPong ? 0 : numBuffers];
}

PAL::DescriptorHandle::NativeHandles ComputeBuffers::getTargetUav(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return (*uavDescriptorRange)[idx + !pingPong ? 0 : numBuffers];
}

ID3D12Resource* ComputeBuffers::getSourceResource(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return buffers[idx + pingPong ? 0 : numBuffers].Get();
}

ID3D12Resource* ComputeBuffers::getTargetResource(unsigned int idx)
{
    BRWL_CHECK(idx < ComputeBuffers::numBuffers, nullptr);
    return buffers[idx + !pingPong ? 0 : numBuffers].Get();
}

BRWL_RENDERER_NS_END
