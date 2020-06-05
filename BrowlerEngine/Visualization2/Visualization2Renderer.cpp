#include "Visualization2Renderer.h"

#include "Core/BrowlerEngine.h"
#include "Common/Logger.h"
#include "Preintegration.h"
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
    pitImage(BRWL_CHAR_LITERAL("Preintegration Table")),
    volumeTexture(),
    pitTexture(),
    textureUploadHeap(nullptr),
    textureFence(nullptr),
    textureFenceEvent(NULL),
    textureFenceLastValue(0),
    initialized(false)
{
    uiResults[0] = {
        {
            UIResult::Settings::Font::OPEN_SANS_REGULAR,
            30
        }, {
            UIResult::TransferFunction::BitDepth::BIT_DEPTH_10_BIT,
            { 0 },
            nullptr
        }
    };
    uiResults[1] = uiResults[0];
    // trigger building the preintegration table
    uiResults[0].transferFunction.bitDepth = UIResult::TransferFunction::BitDepth::BIT_DEPTH_8_BIT;
}

bool Visualization2Renderer::init(Renderer* r)
{
    BRWL_CHECK(!initialized, BRWL_CHAR_LITERAL("Invalid state."));
    
    LoadFonts(uiResults[uiResultIdx].settings.fontSize);
    if (!dataSet.isValid())
    {
        engine->logger->info(BRWL_CHAR_LITERAL("Loading the beatle asset."));
        dataSet.loadFromFile(BRWL_CHAR_LITERAL("./Assets/DataSets/stagbeetle832x832x494.dat"));
        BRWL_EXCEPTION(dataSet.isValid(), BRWL_CHAR_LITERAL("Failed to load default asset."));
    }

    if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&textureFence))), BRWL_CHAR_LITERAL("Failed to create volume texture fence.")))
    {
        return false;
    }

    textureFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!BRWL_VERIFY(textureFenceEvent != NULL, BRWL_CHAR_LITERAL("Failed to create frame fence event.")))
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
    if (v.settings.fontSize != r.settings.fontSize) {
        LoadFonts(r.settings.fontSize);
        v.settings.fontSize = r.settings.fontSize;
    }
}

void Visualization2Renderer::render(Renderer* renderer)
{
    if (!initialized) return;

    uiResultIdx = uiResultIdx ? 0 : 1;
    UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    
    // setting data that is only sent one-way to the front end
    if (pitTexture.state == TextureResource::State::RESIDENT)
    {
        v.transferFunction.textureID = (void*)&pitTexture.descriptorHandle.gpu;
    }

    ImGui::PushFont(fonts[ENUM_CLASS_TO_NUM(r.settings.font)]);
    renderAppUI(r, v);

    if (!pitImage.isValid() || v.transferFunction.bitDepth != r.transferFunction.bitDepth ||
        memcmp(v.transferFunction.transferFunction, r.transferFunction.transferFunction, v.transferFunction.getArrayLenth() * sizeof(UIResult::TransferFunction::sampleT)) !=0)
    {
        //Image image(pitImage.getName());
        const int tableSize = v.transferFunction.getArrayLenth();

        if (!pitImage.isValid() || pitImage.getSizeX() != tableSize)
        {
            pitImage.create(tableSize, tableSize);
        }
        else
        {
            pitImage.clear();
        }

        makePreintegrationTable(pitImage, v.transferFunction.transferFunction, v.transferFunction.getArrayLenth());

        pitTexture.destroy();

        memcpy(v.transferFunction.transferFunction, r.transferFunction.transferFunction, sizeof(v.transferFunction.transferFunction));
        v.transferFunction.bitDepth = r.transferFunction.bitDepth;
    }

    ImGui::PopFont();
}

bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, TextureResource& texture, ComPtr<ID3D12Resource>& textureUploadHeap);
bool LoadFloatTexture2D(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Image* dataSet, TextureResource& texture, ComPtr<ID3D12Resource>& textureUploadHeap);

void Visualization2Renderer::draw(Renderer* r)
{
    if (!initialized) return;

    if (!BRWL_VERIFY(r->srvHeap.isCreated() && r->srvHeap.isShaderVisible() && r->srvHeap.getType() == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, BRWL_CHAR_LITERAL("Invalid descriptor heap.")))
    {
        return;
    }

    if (dataSet.isValid() && volumeTexture.state == TextureResource::State::UNKNOWN)
    {
        uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);

        volumeTexture.descriptorHandle = r->srvHeap.allocateHandle();
        if (!BRWL_VERIFY(LoadVolumeTexture(r->device.Get(), uploadCommandList.Get(), &dataSet, volumeTexture, textureUploadHeap), BRWL_CHAR_LITERAL("Failed to load the volume texture to the GPU.")))
        {   // we expect the function to clean up everything necessary
            return;
        };

        //r->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(volumeTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        uploadCommandList->Close();
        ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
        uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);


        // todo: create multiple instances and check every frame for completion and swap
        // todo: this is killing our usability
        uint64_t fenceValue = textureFenceLastValue + 1;
        uploadCommandQueue->Signal(textureFence.Get(), fenceValue);
        textureFence->SetEventOnCompletion(fenceValue, textureFenceEvent);
        WaitForSingleObject(textureFenceEvent, INFINITE);
        textureFenceLastValue = fenceValue;
        volumeTexture.state = TextureResource::State::RESIDENT;
    }

    if (pitImage.isValid() && pitTexture.state == TextureResource::State::UNKNOWN)
    {
        uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);

        pitTexture.descriptorHandle = r->srvHeap.allocateHandle();
        if (!BRWL_VERIFY(LoadFloatTexture2D(r->device.Get(), uploadCommandList.Get(), &pitImage, pitTexture, textureUploadHeap), BRWL_CHAR_LITERAL("Failed to load the 2D texture to the GPU.")))
        {   // we expect the function to clean up everything necessary
            return;
        };

        //r->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(volumeTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        uploadCommandList->Close();
        ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
        uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        uint64_t fenceValue = textureFenceLastValue + 1;
        uploadCommandQueue->Signal(textureFence.Get(), fenceValue);
        textureFence->SetEventOnCompletion(fenceValue, textureFenceEvent);
        WaitForSingleObject(textureFenceEvent, INFINITE);
        textureFenceLastValue = fenceValue;
        pitTexture.state = TextureResource::State::RESIDENT;
    }
}

void Visualization2Renderer::destroy()
{
    initialized = false;

    volumeTexture.destroy();
    pitTexture.destroy();
    textureUploadHeap = nullptr;
    textureFence = nullptr;
    textureFenceLastValue = 0;
    if (textureFenceEvent)
    {
        CloseHandle(textureFenceEvent);
        textureFenceEvent = nullptr;
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

bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, TextureResource& texture, ComPtr<ID3D12Resource>& textureUploadHeap)
{
    if (!dataSet->isValid() || !BRWL_VERIFY(texture.state == TextureResource::State::UNKNOWN, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))
    {
        return false;
    }

    texture.texture = nullptr;
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
    texDesc.MipLevels = 1;
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
        IID_PPV_ARGS(&texture.texture));

    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture.")))
    {
        texture.texture = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    texture.texture->SetName(dataSet->getName());

    uint64_t requiredSize = GetRequiredIntermediateSize(texture.texture.Get(), 0, 1);
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
        texture.texture = nullptr;
        textureUploadHeap = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    D3D12_SUBRESOURCE_DATA textureData {
        dataSet->getData(),
        dataSet->getStrideY(),
        dataSet->getStrideZ()
    };

    uint64_t result = UpdateSubresources(cmdList, texture.texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
    {
        texture.texture = nullptr;
        textureUploadHeap = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    // Create a SRV for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MipLevels = 1;
    device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, texture.descriptorHandle.cpu);
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
    texture.state = TextureResource::State::LOADING;
    return true;
}

bool LoadFloatTexture2D(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Image* image, TextureResource& texture, ComPtr<ID3D12Resource>& textureUploadHeap)
{
    if (!image->isValid() || !BRWL_VERIFY(texture.state == TextureResource::State::UNKNOWN, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))
    {
        return false;
    }
    // =========================================
    // TODO: reuse upload heaps for upload of preintegration tables
    // =========================================
    texture.texture = nullptr;
    BRWL_EXCEPTION(texture.state == TextureResource::State::UNKNOWN, BRWL_CHAR_LITERAL("Invalid texture resouce state."));
    textureUploadHeap = nullptr;
    D3D12_RESOURCE_DESC texDesc;
    memset(&texDesc, 0, sizeof(texDesc));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = image->getSizeX();
    texDesc.Height = image->getSizeY();
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32_FLOAT;
    texDesc.SampleDesc = { 1, 0 };
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // todo: try swizzled texture, may be faster
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&texture.texture));

    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture.")))
    {
        texture.texture = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    texture.texture->SetName(image->getName());

    uint64_t requiredSize = GetRequiredIntermediateSize(texture.texture.Get(), 0, 1);
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
        texture.texture = nullptr;
        textureUploadHeap = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    D3D12_SUBRESOURCE_DATA textureData {
        image->getData(),
        image->getStrideY(),
        image->getBufferSize()
    };

    uint64_t result = UpdateSubresources(cmdList, texture.texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
    {
        texture.texture = nullptr;
        textureUploadHeap = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    // Create a SRV for the texture
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.PlaneSlice = 0;
    device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, texture.descriptorHandle.cpu);

    texture.state = TextureResource::State::LOADING;
    return true;
}



BRWL_RENDERER_NS_END
