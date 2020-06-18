#include "ComputeBuffers.h"
#include "Core/BrowlerEngine.h"
#include "Common/Logger.h"

BRWL_RENDERER_NS


ComputeBuffers::ComputeBuffers() :
    bufferWidth(0),
    bufferHeight(0),
    srvBuffers{ nullptr },
    uavBuffers{ nullptr },
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

    D3D12_RESOURCE_DESC textureDecriptions[numLayers * numBuffers];
    memset(&textureDecriptions, 0, sizeof(textureDecriptions));
    textureDecriptions[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDecriptions[0].Alignment = 0; // should choose D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT automatically
    textureDecriptions[0].Width = bufferWidth;
    textureDecriptions[0].Height = bufferHeight;
    textureDecriptions[0].DepthOrArraySize = 1;
    textureDecriptions[0].MipLevels = 1;
    textureDecriptions[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // with alpha because we are rich (and we bind it as a uav which doesn't support RGB-only
    textureDecriptions[0].SampleDesc = { 1, 0 };
    textureDecriptions[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDecriptions[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;

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
        if (!BRWL_VERIFY(SUCCEEDED(device->CreatePlacedResource(bufferHeap.Get(), info.Offset, &textureDecriptions[i], D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&srvBuffers[i]))), BRWL_CHAR_LITERAL("Failed to place resource into heap.")))
        {
            destroy();
            return false;
        }
        srvBuffers[i]->SetName(L"ALIASED COMPUTE SRV");

        if (!BRWL_VERIFY(SUCCEEDED(device->CreatePlacedResource(bufferHeap.Get(), info.Offset, &textureDecriptions[i], D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uavBuffers[i]))), BRWL_CHAR_LITERAL("Failed to place resource into heap.")))
        {
            destroy();
            return false;
        }
        uavBuffers[i]->SetName(L"ALIASED COMPUTE UAV");
    }

    // Create UAVs && SRVs for the textures
    srvDescriptorRange = srvHeap->allocateRange(countof(srvBuffers), BRWL_CHAR_LITERAL("SRV COMPUTE BUFFERS"));
    uavDescriptorRange = srvHeap->allocateRange(countof(uavBuffers), BRWL_CHAR_LITERAL("UAV COMPUTE BUFFERS"));
    for (int i = 0; i < countof(allocInfoEx); ++i)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        memset(&srvDesc, 0, sizeof(srvDesc));
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDecriptions[i].Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.PlaneSlice = 0;
        device->CreateShaderResourceView(srvBuffers[i].Get(), &srvDesc, (*srvDescriptorRange)[i].cpu);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        memset(&uavDesc, 0, sizeof(uavDesc));
        uavDesc.Format = textureDecriptions[i].Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
        device->CreateUnorderedAccessView(uavBuffers[i].Get(), nullptr, &uavDesc, (*uavDescriptorRange)[i].cpu);
    }

    created = true;
    return created;
}

void ComputeBuffers::destroy()
{
    created = false;

    bufferWidth = 0;
    bufferHeight = 0;
    for (int i = 0; i < countof(*(decltype(uavBuffers)*)(0)); ++i)
    {
        uavBuffers[i] = nullptr;
    }

    for (int i = 0; i < countof(*(decltype(srvBuffers)*)(0)); ++i)
    {
        srvBuffers[i] = nullptr;
    }

    bufferHeap = nullptr;
    srvDescriptorRange = nullptr;
    uavDescriptorRange = nullptr;
}

bool ComputeBuffers::isResident() const
{
    return srvDescriptorRange && srvDescriptorRange->isResident() && uavDescriptorRange && uavDescriptorRange->isResident();
}

void ComputeBuffers::swap(ID3D12GraphicsCommandList* cmd)
{
    // invert ping-pong variable
    pingPong = !pingPong;

    // we now made all UAVs SRVs and all SRVs became UAVs
    D3D12_RESOURCE_BARRIER barriers[numBuffers * numLayers];
    memset(&barriers, 0, sizeof(barriers));
    for (int i = 0; i < numBuffers; ++i)
    {
        barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barriers[i].Aliasing.pResourceBefore = getSrvResource(i, true);
        barriers[i].Aliasing.pResourceAfter = getUavResource(i);
    }

    for (int i = 0; i < numBuffers; ++i)
    {
        barriers[numBuffers + i].Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        barriers[numBuffers + i].Aliasing.pResourceBefore = getUavResource(i, true);
        barriers[numBuffers + i].Aliasing.pResourceAfter = getSrvResource(i);
    }

    cmd->ResourceBarrier(countof(barriers), barriers);
}

PAL::DescriptorHandle::NativeHandles ComputeBuffers::getSourceSrv(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return (*srvDescriptorRange)[idx + (pingPong ? 0 : numBuffers)];
}

PAL::DescriptorHandle::NativeHandles ComputeBuffers::getTargetUav(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return (*uavDescriptorRange)[idx + (!pingPong ? 0 : numBuffers)];
}

ID3D12Resource* ComputeBuffers::getSrvResource(unsigned int idx, bool before)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return srvBuffers[idx + (pingPong ^ before ? 0 : numBuffers)].Get();
}

ID3D12Resource* ComputeBuffers::getUavResource(unsigned int idx, bool before)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return uavBuffers[idx + (!pingPong ^ before ? 0 : numBuffers)].Get();
}

BRWL_RENDERER_NS_END
