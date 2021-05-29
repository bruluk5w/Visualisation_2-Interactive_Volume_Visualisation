#include "ComputeBuffers.h"

#include "Core/BrowlerEngine.h"
#include "Renderer/PAL/DX12Helpers.h"


BRWL_RENDERER_NS

namespace {
    const LPCWSTR computeNames[ComputeBuffers::numLayers * ComputeBuffers::numBuffers] = {
        L"COMPUTE UAV/SRV 0",
        L"COMPUTE UAV/SRV 1",
        L"COMPUTE UAV/SRV 2",
        L"COMPUTE UAV/SRV 3",
        L"COMPUTE UAV/SRV 4",
        L"COMPUTE UAV/SRV 5",
        L"COMPUTE UAV/SRV 6",
        L"COMPUTE UAV/SRV 7",
        L"COMPUTE UAV/SRV 8",
        L"COMPUTE UAV/SRV 9",
        L"COMPUTE UAV/SRV 10",
        L"COMPUTE UAV/SRV 11",
    };
}
#define USE_UAV_SPLIT_BARRIER 0  // althrough spec notes that this should be possible in the future, at least the current game ready driver throws an error

ComputeBuffers::ComputeBuffers() :
    bufferWidth(0),
    bufferHeight(0),
    resources{ nullptr },
    bufferHeap(nullptr),
    uavDescriptorRange(nullptr),
    created(false),
    pingPong(false),
    uavUseSet(false),
    colorTexUsedByPixelShader(false)
#if USE_UAV_SPLIT_BARRIER
    , uavBarrierActive(false)
#endif
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
    textureDecriptions[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    // for now we use the same format for all buffers 
    for (int i = 1; i < 6; ++i) {
        textureDecriptions[i] = textureDecriptions[0];
    }
    // copy to 2nd layer
    for (int i = 0; i < 6; ++i) {
        textureDecriptions[6 + i] = textureDecriptions[i];
    }

    // how much memory should we allocate?
    ComPtr<ID3D12Device4> device4;
    if (!BRWL_VERIFY(SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&device4))), nullptr))
    {
        destroy(srvHeap);
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
    heapDesc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;


    if (!BRWL_VERIFY(SUCCEEDED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&bufferHeap))), BRWL_CHAR_LITERAL("Failed to allocate compute buffers.")))
    {
        destroy(srvHeap);
        return false;
    }

    //Place resources
    for (int i = 0; i < countof(allocInfoEx); ++i)
    {
        D3D12_RESOURCE_ALLOCATION_INFO1& info = allocInfoEx[i];
        if (!BRWL_VERIFY(SUCCEEDED(device->CreatePlacedResource(bufferHeap.Get(), info.Offset, &textureDecriptions[i], D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resources[i]))), BRWL_CHAR_LITERAL("Failed to place resource into heap.")))
        {
            destroy(srvHeap);
            return false;
        }

        resources[i]->SetName(computeNames[i]);
    }

    // Create UAVs for all textures
    uavDescriptorRange = srvHeap->allocateRange(countof(resources), BRWL_CHAR_LITERAL("UAV COMPUTE BUFFERS"));

    for (int i = 0; i < countof(allocInfoEx); ++i)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        memset(&uavDesc, 0, sizeof(uavDesc));
        uavDesc.Format = textureDecriptions[i].Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
        device->CreateUnorderedAccessView(resources[i].Get(), nullptr, &uavDesc, uavDescriptorRange->getNonResident(i).cpu);
    }

    srvDescriptorRange = srvHeap->allocateRange(numLayers * numSrvReadBuffers, BRWL_CHAR_LITERAL("UAV COMPUTE BUFFERS"));
    // Create SRVs for textures that need SampleLevel when bound as the read layer
    for (int i = 0; i < numLayers; ++i)
    {
        for (int j = 0; j < numSrvReadBuffers; ++j)
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
            memset(&srvDesc, 0, sizeof(srvDesc));
            srvDesc.Format = textureDecriptions[i].Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.PlaneSlice = 0;
            device->CreateShaderResourceView(resources[i * numBuffers + srvReadBufferOffset + j].Get(), &srvDesc, srvDescriptorRange->getNonResident(i * numSrvReadBuffers + j).cpu);
        }
    }

    created = true;
    return created;
}

void ComputeBuffers::setInitialResourceState(ID3D12GraphicsCommandList* cmd)
{
    if (!uavUseSet)
    {
        // if pingPong is true then the read buffers are the first numBuffer resources
        const int srvOffset = pingPong ? 0 : numBuffers;

        // for the write layer we have to transition all resources to be used as UAVs since they need write access
        ID3D12Resource* uavResources[numBuffers * numLayers - numSrvReadBuffers];
        int idx = 0;
        for (int i = 0; i < countof(uavResources); ++i)
        {
            // for the read layer we exclude the SRV read buffers
            if (idx == srvOffset + srvReadBufferOffset)
                idx = srvOffset + srvReadBufferOffset + numSrvReadBuffers;

            uavResources[i] = resources[idx].Get();
            ++idx;
        }

        BRWL_CHECK(idx == numLayers * numBuffers, nullptr);

        PAL::stateTransition<D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS>(cmd, uavResources);
        
        // transition subset that we will need to read as SRVs
        ID3D12Resource* srvResources[numSrvReadBuffers];
        for (int i = 0; i < numSrvReadBuffers; ++i)
        {
            srvResources[i] = resources[i + srvOffset + srvReadBufferOffset].Get();
        }

        PAL::stateTransition<D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE>(cmd, srvResources);
        afterComputeUse(cmd);
        uavUseSet = true;
    }
}

void ComputeBuffers::destroy(PAL::DescriptorHeap* srvHeap)
{
    created = false;

    bufferWidth = 0;
    bufferHeight = 0;
    for (int i = 0; i < countof(*(decltype(resources)*)(0)); ++i)
    {
        resources[i] = nullptr;
    }

    bufferHeap = nullptr;
    {
        PAL::ResourceFrame f(srvHeap);
        srvHeap->releaseRange(uavDescriptorRange);
        uavDescriptorRange = nullptr;
        srvHeap->releaseRange(srvDescriptorRange);
        srvDescriptorRange = nullptr;
    }
    uavUseSet = false;
    colorTexUsedByPixelShader = false;
}

bool ComputeBuffers::isResident() const
{
    return uavDescriptorRange && uavDescriptorRange->isResident();
}


void ComputeBuffers::closeUavBarrier(ID3D12GraphicsCommandList* cmd)
{
#if USE_UAV_SPLIT_BARRIER
    if (uavBarrierActive)
    {
        D3D12_RESOURCE_BARRIER barriers[numBuffers];
        memset(&barriers, 0, sizeof(barriers));
        for (int i = 0; i < numBuffers; ++i)
        {
            barriers[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barriers[i].UAV.pResource = getSourceResource(i, true);
        }

        cmd->ResourceBarrier(countof(barriers), barriers);
        uavBarrierActive = false;
    }
#endif
}

void ComputeBuffers::beforeComputeUse(ID3D12GraphicsCommandList* cmd)
{
    BRWL_CHECK(!colorTexUsedByPixelShader, nullptr);

    // swap previous source resources with previous target resources
    pingPong = !pingPong;

    // if pingPong is true then the read buffers are the first numBuffer resources
    const int currentSrvOffset = pingPong ? 0 : numBuffers;
    // end transitioning subset of resources that we need to read as SRVs
    ID3D12Resource* srvResources[numSrvReadBuffers];
    for (int i = 0; i < numSrvReadBuffers; ++i)
    {
        srvResources[i] = resources[i + currentSrvOffset + srvReadBufferOffset].Get();
    }

    PAL::stateTransition<D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, PAL::Split::END>(cmd, srvResources);


    const int perviousSrvOffset = pingPong ? numBuffers : 0;
    // end transitioning subset that were SRVs before but we need to write to them now
    for (int i = 0; i < numSrvReadBuffers; ++i)
    {
        srvResources[i] = resources[i + perviousSrvOffset + srvReadBufferOffset].Get();
    }

    PAL::stateTransition<D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, PAL::Split::END>(cmd, srvResources);

    // make sure all writes are complete on the previous write layer that is now the read layer
    // ?for the resources that have been transitioned to SRVs this is maybe superfluous?
#if USE_UAV_SPLIT_BARRIER
    closeUavBarrier(cmd);
#else
    D3D12_RESOURCE_BARRIER barriers[numBuffers];
    memset(&barriers, 0, sizeof(barriers));
    for (int i = 0; i < numBuffers; ++i)
    {
        barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barriers[i].UAV.pResource = getSourceResource(i);
    }

    cmd->ResourceBarrier(countof(barriers), barriers);
#endif
}

void ComputeBuffers::afterComputeUse(ID3D12GraphicsCommandList* cmd)
{
    // swap previous source resources with previous target resources
#if USE_UAV_SPLIT_BARRIER
    D3D12_RESOURCE_BARRIER barriers[numBuffers];
    memset(&barriers, 0, sizeof(barriers));
    for (int i = 0; i < numBuffers; ++i)
    {
        barriers[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
        barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barriers[i].UAV.pResource = getSourceResource(i, true);
    }

    cmd->ResourceBarrier(countof(barriers), barriers);
    uavBarrierActive = true;
#endif

    // if pingPong is true then the read buffers are the first numBuffer resources
    const int futureSrvOffset = pingPong ? numBuffers : 0;
    // start transitioning subset that we will need to read as SRVs
    ID3D12Resource* srvResources[numSrvReadBuffers];
    for (int i = 0; i < numSrvReadBuffers; ++i)
    {
        srvResources[i] = resources[i + futureSrvOffset + srvReadBufferOffset].Get();
    }

    PAL::stateTransition<D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, PAL::Split::BEGIN>(cmd, srvResources);

    const int currentSrvOffset = pingPong ? 0 : numBuffers;
    // start transitioning subset that are now SRVs but we need to write to them in the future
    for (int i = 0; i < numSrvReadBuffers; ++i)
    {
        srvResources[i] = resources[i + currentSrvOffset + srvReadBufferOffset].Get();
    }

    PAL::stateTransition<D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, PAL::Split::BEGIN>(cmd, srvResources);

}

PAL::DescriptorHandle::ResidentHandles ComputeBuffers::getTargetResourceDescriptorHandle(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return uavDescriptorRange->getResident(idx + (!pingPong ? 0 : numBuffers));
}

PAL::DescriptorHandle::ResidentHandles ComputeBuffers::getSourceUavResourceDescriptorHandle(unsigned int idx)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return uavDescriptorRange->getResident(idx + (pingPong ? 0 : numBuffers));
}

PAL::DescriptorHandle::ResidentHandles ComputeBuffers::getSourceSrvResourceDescriptorHandle(unsigned int idx)
{
    BRWL_CHECK(idx < numSrvReadBuffers, nullptr);
    return srvDescriptorRange->getResident(idx + (pingPong ? 0 : numSrvReadBuffers));
}

ID3D12Resource* ComputeBuffers::getTargetResource(unsigned int idx, bool before)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return resources[idx + (!pingPong ^ before ? 0 : numBuffers)].Get();
}

ID3D12Resource* ComputeBuffers::getSourceResource(unsigned int idx, bool before)
{
    BRWL_CHECK(idx < numBuffers, nullptr);
    return resources[idx + (pingPong ^ before ? 0 : numBuffers)].Get();
}

BRWL_RENDERER_NS_END
