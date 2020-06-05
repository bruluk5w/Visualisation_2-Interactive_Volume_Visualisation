#include "Visualization2Renderer.h"

#include "Core/BrowlerEngine.h"
#include "Common/Logger.h"
#include "Renderer/Renderer.h"
#include "Renderer/PAL/imgui_impl_dx12.h"
#include "Renderer/PAL/d3dx12.h"

BRWL_RENDERER_NS


Visualization2Renderer::Visualization2Renderer() :
    AppRenderer(),
    uiResultIdx(0),
    fonts{ 0 },
    uploadCommandQueue(nullptr),
    uploadCommandAllocator(nullptr),
    uploadCommandList(nullptr),
    dataSet(BRWL_CHAR_LITERAL("Default Data Sets (Beetle)")),
    volumeTexture(nullptr),
    volumeTextureUploadHeap(nullptr),
    volumeTextureFence(nullptr),
    volumeTextureFenceEvent(NULL),
    volumeTextureFenceLastValue(0),
    initialized(false)
{
    uiResults[0] = { UIResult::Font::OPEN_SANS_REGULAR, 30 };
    uiResults[1] = uiResults[0];
}

bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUploadHeap);

bool Visualization2Renderer::init(Renderer* r)
{
    BRWL_CHECK(!initialized, BRWL_CHAR_LITERAL("Invalid state."));
    
    LoadFonts(uiResults[uiResultIdx].fontSize);
    if (!dataSet.isValid())
    {
        engine->logger->info(BRWL_CHAR_LITERAL("Loading the beatle asset."));
        dataSet.loadFromFile(BRWL_CHAR_LITERAL("./Assets/DataSets/stagbeetle832x832x494.dat"));
        BRWL_EXCEPTION(dataSet.isValid(), BRWL_CHAR_LITERAL("Failed to load default asset."));
    }

    if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&volumeTextureFence))), BRWL_CHAR_LITERAL("Failed to create volume texture fence.")))
    {
        return false;
    }

    volumeTextureFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!BRWL_VERIFY(volumeTextureFenceEvent != NULL, BRWL_CHAR_LITERAL("Failed to create frame fence event.")))
    {
        return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateCommandQueue(&desc, IID_PPV_ARGS(&uploadCommandQueue))), BRWL_CHAR_LITERAL("Failed to create upload command queue.")))
        {
            return false;
        }
    }

    if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(uploadCommandAllocator.ReleaseAndGetAddressOf()))), BRWL_CHAR_LITERAL("Failed to create direct command allocator.")))
    {
        return false;
    }

    if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, uploadCommandAllocator.Get(), NULL, IID_PPV_ARGS(&uploadCommandList))), BRWL_CHAR_LITERAL("Failed to create upload command list.")))
    {
        return false;
    }

    if (!BRWL_VERIFY(SUCCEEDED(uploadCommandList->Close()), BRWL_CHAR_LITERAL("Failed to close upload command list.")))
    {
        return false;
    }

    initialized = true;

    return initialized;
}

void Visualization2Renderer::preRender(Renderer* renderer)
{
    if (!initialized) return;

    const UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    if (v.fontSize != r.fontSize) {
        LoadFonts(r.fontSize);
        v.fontSize = r.fontSize;
    }
}

void Visualization2Renderer::render(Renderer* renderer)
{
    if (!initialized) return;

    uiResultIdx = uiResultIdx ? 0 : 1;
    UIResult& r = uiResults[uiResultIdx]; // results
    const UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    ImGui::PushFont(fonts[ENUM_CLASS_TO_NUM(r.font)]);
    renderAppUI(r, v);
    ImGui::PopFont();
}
void Visualization2Renderer::draw(Renderer* r)
{
    if (!initialized) return;

    // volumeTextureFenceLastValue == 0 --> only upload once
    if (dataSet.isValid() && volumeTextureFenceLastValue == 0)
    {
        uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);
        if (!BRWL_VERIFY(LoadVolumeTexture(r->device.Get(), uploadCommandList.Get(), &dataSet, volumeTexture, volumeTextureUploadHeap), BRWL_CHAR_LITERAL("Failed to load the volume texture to the GPU.")))
        {
            return;
        };

        //r->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(volumeTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        uploadCommandList->Close();

        ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
        uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        // todo: create multiple instances and check every frame for completion and swap
        // todo: this is killing our usability
        UINT64 fenceValue = volumeTextureFenceLastValue + 1;
        uploadCommandQueue->Signal(volumeTextureFence.Get(), fenceValue);
        volumeTextureFence->SetEventOnCompletion(fenceValue, volumeTextureFenceEvent);
        WaitForSingleObject(volumeTextureFenceEvent, INFINITE);
        volumeTextureFenceLastValue = fenceValue;


    }
}

void Visualization2Renderer::destroy()
{
    initialized = false;

    volumeTexture = nullptr;
    volumeTextureUploadHeap = nullptr;
    volumeTextureFence = nullptr;
    volumeTextureFenceLastValue = 0;
    if (volumeTextureFenceEvent)
    {
        CloseHandle(volumeTextureFenceEvent);
        volumeTextureFenceEvent = nullptr;
    }
    uploadCommandAllocator = nullptr;
    uploadCommandList = nullptr;
    uploadCommandQueue = nullptr;
}

void Visualization2Renderer::LoadFonts(float fontSize)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    fonts[0] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Light.ttf", fontSize);
    fonts[1] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Regular.ttf", fontSize);
    fonts[2] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Semibold.ttf", fontSize);
    fonts[3] = io.Fonts->AddFontFromFileTTF("./Assets/Fonts/OpenSans-Bold.ttf", fontSize);
    ImGui_ImplDX12_CreateFontsTexture();
}

bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, ComPtr<ID3D12Resource>& texture, ComPtr<ID3D12Resource>& textureUploadHeap) {
    if (!dataSet->isValid()) return false;
    texture = nullptr;
    textureUploadHeap = nullptr;
    // =========================================
    // TODO: add a second path for 8bit textures
    // =========================================
    D3D12_RESOURCE_DESC texDesc;
    memset(&texDesc, 0, sizeof(texDesc));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    texDesc.Alignment = 0;
    texDesc.Width = dataSet->getSizeX();
    texDesc.Height = dataSet->getSizeY();
    texDesc.DepthOrArraySize= dataSet->getSizeZ();
    texDesc.MipLevels = 0;
    texDesc.Format = DXGI_FORMAT_R16_UNORM;
    texDesc.SampleDesc = { 1,0 };
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&texture));

    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture.")))
    {
        texture = nullptr;
        return false;
    }

    texture->SetName(L"Main Volume Texture");

    uint64_t requiredSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);
    // Create the GPU upload buffer.
    hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&textureUploadHeap));
    

    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture upload heap.")))
    {
        texture = nullptr;
        textureUploadHeap = nullptr;
        return false;
    }

    D3D12_SUBRESOURCE_DATA textureData {
        dataSet->getData(),
        dataSet->getStrideY(),
        dataSet->getStrideZ()
    };

    uint64_t result = UpdateSubresources(cmdList, texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
    {
        texture = nullptr;
        textureUploadHeap = nullptr;
        return false;
    }

    // Create a SRV for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MipLevels = 1;
    device->CreateShaderResourceView(texture.Get(), &srvDesc, srvHeap->GetCPUDescriptorHandleForHeapStart());
     //hr = device->CreateCommittedResource(
     //   &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
     //   D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES,  // set to none if this is a problem
     //   &texDesc,
     //   D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
     //   IID_PPV_ARGS(&texture));


    //CD3DX12_RANGE readRange(0, 0); // No CPU read
    //uint8_t* cpuPtr;
    //{
    //    void* ptr;
    //    hr = texture->Map(0, &readRange, &ptr);
    //    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to fetch CPU pointer for volume texture.")))
    //    {
    //        texture = nullptr;
    //        return;
    //    }

    //    cpuPtr = (uint8_t*)ptr;
    //}

    //memcpy(cpuPtr, dataSet->getData(), dataSet->getBufferSize());

    //m_pDataCur = m_pDataBegin = reinterpret_cast<UINT8*>(pData);
    //m_pDataEnd = m_pDataBegin + uSize;
    return true;
}


BRWL_RENDERER_NS_END
