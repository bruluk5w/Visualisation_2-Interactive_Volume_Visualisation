#include "Visualization2Renderer.h"

#include "Common/Logger.h"
#include "Common/BoundingBox.h"
#include "Core/BrowlerEngine.h"
#include "Preintegration.h"
#include "Renderer/PAL/imgui_impl_dx12.h"
#include "Renderer/PAL/d3dx12.h"

BRWL_RENDERER_NS


Visualization2Renderer::Visualization2Renderer() :
    AppRenderer(),
    uiResultIdx(0),
    uiResults{ {},{} },
    fonts{ 0 },
    uploadCommandQueue(nullptr),
    uploadCommandAllocator(nullptr),
    uploadCommandList(nullptr),
    dataSet(BRWL_CHAR_LITERAL("Default Data Sets (Beetle)")),
    pitCollection(),
    volumeTexture(),
    volumeTextureUploadFence(nullptr),
    uploadFenceEvent(NULL),
    volumeTextureFenceValue(0),
    mainShader(),
    initialized(false)
{
    // trigger building the preintegration tables
    for (int i = 0; i < countof(((UIResult::TransferFunctionCollection*)0)->array); ++i)
    {
        UIResult::TransferFunction& tFunc = uiResults[0].transferFunctions.array[i];
        const bool is8bit = tFunc.bitDepth == UIResult::TransferFunction::BitDepth::BIT_DEPTH_8_BIT;
        tFunc.bitDepth = is8bit ? UIResult::TransferFunction::BitDepth::BIT_DEPTH_10_BIT : UIResult::TransferFunction::BitDepth::BIT_DEPTH_8_BIT;
    }
}

bool Visualization2Renderer::init(Renderer* r)
{
    BRWL_CHECK(!initialized, BRWL_CHAR_LITERAL("Invalid state."));
    
    LoadFonts(uiResults[uiResultIdx].settings.fontSize);
    if (!dataSet.isValid())
    {
        engine->logger->info(BRWL_CHAR_LITERAL("Loading the beatle asset."));
        dataSet.loadFromFile(BRWL_CHAR_LITERAL("./Assets/DataSets/stagbeetle832x832x494.dat"));
        if (BRWL_VERIFY(dataSet.isValid(), BRWL_CHAR_LITERAL("Failed to load default asset.")))
        {
            volumeTexture.state = TextureResource::State::REQUESTING_UPLOAD;
        }
    }

    if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&volumeTextureUploadFence))), BRWL_CHAR_LITERAL("Failed to create volume texture fence.")))
    {
        return false;
    }
    
    pitCollection.init(r->device.Get());

    uploadFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!BRWL_VERIFY(uploadFenceEvent != NULL, BRWL_CHAR_LITERAL("Failed to create frame fence event.")))
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

        uploadCommandQueue->SetName(L"Upload Command Queue");
    }

    if (!BRWL_VERIFY(SUCCEEDED(r->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&uploadCommandAllocator))), BRWL_CHAR_LITERAL("Failed to create direct command allocator.")))
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

    uploadCommandList->SetName(L"Upload Command List");

    if (!BRWL_VERIFY(mainShader.create(r->device.Get()), BRWL_CHAR_LITERAL("Failed to create main shader.")))
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

    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        PitImage& pitImage = pitCollection.array[i];
        const uint64_t completed = pitImage.fence->GetCompletedValue();

        // swap texture if ready
        if (pitImage.stagedTexture->state == TextureResource::State::LOADING && completed >= pitImage.uploadFenceValue)
        {
            pitImage.stagedTexture->state = TextureResource::State::RESIDENT;
            pitImage.stagedTexture->uploadHeap = nullptr; // free some resources
            // We might still have some frames in flight with the old texture
            // We have to wait until we are sure we that are the only one tampering with resources
            renderer->waitForLastSubmittedFrame();
            std::swap(pitImage.stagedTexture, pitImage.liveTexture);
            // release old texture
            pitImage.stagedTexture->destroy();
            // Indicate new resource to be used by the pixel shader on the main command queue.
            // We only do this once after the upload. In the next frames the resource will use implicit state promotion between command and pixel shader resource.
            renderer->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pitImage.liveTexture->texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        }
    }

    // Swap objects fro communication with front-end
    uiResultIdx = uiResultIdx ? 0 : 1;
    UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values

    // set textures to the front-end
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        const PitImage& pitImage = pitCollection.array[i];
        v.transferFunctions.array[i].textureID = pitImage.liveTexture->state != TextureResource::State::RESIDENT ? nullptr : (ImTextureID)pitImage.liveTexture->descriptorHandle.gpu.ptr;
    }

    // execute UI and retrieve results
    ImGui::PushFont(fonts[ENUM_CLASS_TO_NUM(r.settings.font)]);
    renderAppUI(r, v);
    ImGui::PopFont();

    // Check if textures should be recomputed
    bool mustRecompute[countof(*((decltype(pitCollection.array)*)nullptr))] = { false };
    bool blocked[countof(mustRecompute)] = { false };
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        PitImage& pitImage = pitCollection.array[i];
        UIResult::TransferFunction& tFuncValue = v.transferFunctions.array[i];
        UIResult::TransferFunction& tFuncResult = r.transferFunctions.array[i];
        // TODO: assert on dissimilar bitDepth!
        if (!pitImage.cpuImage.isValid() || tFuncValue.bitDepth != tFuncResult.bitDepth ||
            memcmp(tFuncValue.transferFunction, tFuncResult.transferFunction, tFuncValue.getArrayLength() * sizeof(UIResult::TransferFunction::sampleT)) != 0)
        {
            mustRecompute[i] = true;
            // If staged texture is not free, wait for upload to complete
            if (pitImage.stagedTexture->state != TextureResource::State::UNKNOWN)
            {
                blocked[i] = true;
                // if the staged resource is currently upoading then the fence value has do be lower than the one which we remembered
                // but only do a weak check since it could theoretically finish in this very moment
                BRWL_CHECK(pitImage.fence->GetCompletedValue() < pitImage.uploadFenceValue, BRWL_CHAR_LITERAL("Invalid fence/staged resource state."));
                if (!BRWL_VERIFY(pitImage.stagedTexture->state != TextureResource::State::FAILED, BRWL_CHAR_LITERAL("Invalid state for staged pitTexture.")))
                {
                    continue;
                }

                // prepare for waiting 
                pitImage.fence->SetEventOnCompletion(pitImage.uploadFenceValue, pitImage.uploadEvent);
            }
        }
    }

    // gather all events we still have to wait for
    HANDLE waitHandles[countof(blocked)] = { NULL };
    unsigned int numWaithandles = 0;
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        if (blocked[i])
        {
            PitImage& pitImage = pitCollection.array[i];
            waitHandles[numWaithandles] = pitImage.uploadEvent;
            ++numWaithandles;
        }
    }

    // TODO: meanwhile we could already recompute the non-blocked tables

    // wait and clean up event state 
    WaitForMultipleObjects(numWaithandles, waitHandles, true, INFINITE);

    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        if (blocked[i])
        {
            PitImage& pitImage = pitCollection.array[i];
            ResetEvent(pitImage.uploadEvent);
            // Since the live texture is currently in use in the front-end we do not swap but immediately recycle the staged texture.
            // This means that a texture change is only visible to the user if there is one frame without a change that requires recomputation.
            pitImage.stagedTexture->destroy();
        }
    }


    // Recompute
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        if(mustRecompute[i])
        {
            PitImage& pitImage = pitCollection.array[i];
            UIResult::TransferFunction& tFuncValue = v.transferFunctions.array[i];
            UIResult::TransferFunction& tFuncResult = r.transferFunctions.array[i];
            // Recompute texture
            const int tableSize = tFuncResult.getArrayLength();

            if (!pitImage.cpuImage.isValid() || pitImage.cpuImage.getSizeX() != tableSize)
                pitImage.cpuImage.create(tableSize, tableSize);
            else
                pitImage.cpuImage.clear();

            makePreintegrationTable(pitImage.cpuImage, tFuncResult.transferFunction, tFuncResult.getArrayLength());

            // upload in draw()
            // pitImage.stagedTexture->descriptorHandle = renderer->srvHeap.allocateHandle(
//#ifdef _DEBUG
//            BRWL_CHAR_LITERAL("StagedPitTexture")
//#endif
//            );
            pitImage.stagedTexture->state = TextureResource::State::REQUESTING_UPLOAD;

            // set front-end request satisfied
            memcpy(tFuncValue.transferFunction, tFuncResult.transferFunction, sizeof(tFuncValue.transferFunction));
            tFuncValue.bitDepth = tFuncResult.bitDepth;
            tFuncValue.controlPoints = tFuncResult.controlPoints;
        }
    }
    if (v.settings.vsync != r.settings.vsync)
    {
        renderer->setVSync(r.settings.vsync);
        v.settings.vsync = r.settings.vsync;
    }

    // no action needed
    v.settings.voxelsPerCm = r.settings.voxelsPerCm;
    v.settings.font = r.settings.font;
    v.settings.drawAssetBoundaries = r.settings.drawAssetBoundaries;
    v.settings.drawViewingVolumeBoundaries = r.settings.drawViewingVolumeBoundaries;

    mainShader.render();
}

bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, TextureResource& texture);
bool LoadFloatTexture2D(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Image* image, TextureResource& texture);

#define DR(expression) PAL::HandleDeviceRemoved(expression, r->device.Get(), *r->logger)

void Visualization2Renderer::draw(Renderer* r)
{
    if (!initialized) return;

    if (!BRWL_VERIFY(r->srvHeap.isCreated() && r->srvHeap.isShaderVisible() && r->srvHeap.getType() == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, BRWL_CHAR_LITERAL("Invalid descriptor heap.")))
    {
        return;
    }

    // we always need the volume texture so ensure we have it first.
    // this stalls the pipeline until the copy is complete but maybe we don't care 
    // because this is the data we want to visualize and if we don't have it we see nothing anyways
    if (volumeTexture.state == TextureResource::State::REQUESTING_UPLOAD)
    {
        BRWL_EXCEPTION(dataSet.isValid(), BRWL_CHAR_LITERAL("Invalid state of data set."));
        uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);
        
        // Since the volume texture is not double buffered, we have to wait unil all frames still rendering from the volume texture have finished
        r->waitForLastSubmittedFrame();

        volumeTexture.descriptorHandle = r->srvHeap.allocateHandle(
#ifdef _DEBUG
            BRWL_CHAR_LITERAL("VolumeTexture")
#endif
        );
        volumeTexture.state = TextureResource::State::REQUESTING_UPLOAD;
        if (!BRWL_VERIFY(LoadVolumeTexture(r->device.Get(), uploadCommandList.Get(), &dataSet, volumeTexture), BRWL_CHAR_LITERAL("Failed to load the volume texture to the GPU.")))
        {   // we expect the function to clean up everything necessary
            return;
        };

        DR(uploadCommandList->Close());
        ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
        uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        // wait synchronously for the upload to complete
        ++volumeTextureFenceValue;
        DR(uploadCommandQueue->Signal(volumeTextureUploadFence.Get(), volumeTextureFenceValue));
        DR(volumeTextureUploadFence->SetEventOnCompletion(volumeTextureFenceValue, uploadFenceEvent));
        WaitForSingleObject(uploadFenceEvent, INFINITE);
        volumeTexture.state = TextureResource::State::RESIDENT;
        volumeTexture.uploadHeap = nullptr;  // free resources

        // indicate new resource to be used by the pixel shader on the main command queue
        r->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(volumeTexture.texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    }

    // If we have all resources then we draw else we wait another frame
    if (pitCollection.isResident())
    {
        const MainShader::DrawData drawData {
            &dataSet.getBoundingBox(),
            &volumeTexture,
            &pitCollection,
            uiResults[0].settings.voxelsPerCm,
            uiResults[0].settings.drawAssetBoundaries,
            uiResults[0].settings.drawViewingVolumeBoundaries,
        };
        mainShader.draw(r->device.Get(), r->commandList.Get(), drawData);
    }

    // check if we have to start uploading textures
    bool hasUpload = std::any_of(pitCollection.array, pitCollection.array + countof(pitCollection.array), [](const PitImage& p) {
        return p.stagedTexture->state == TextureResource::State::REQUESTING_UPLOAD;
        });

    if (hasUpload)
    {
        DR(uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr));
        bool uploading[countof(*((decltype(pitCollection.array)*)nullptr))] = { false };

        for (int i = 0; i < countof(pitCollection.array); ++i)
        {
            PitImage& pitImage = pitCollection.array[i];
            if (pitImage.stagedTexture->state == TextureResource::State::REQUESTING_UPLOAD)
            {
                uploading[i] = true;
                pitImage.stagedTexture->descriptorHandle = r->srvHeap.allocateHandle(
#ifdef _DEBUG
                    BRWL_CHAR_LITERAL("StagedPitTexture")
#endif
                );
                if (!BRWL_VERIFY(LoadFloatTexture2D(r->device.Get(), uploadCommandList.Get(), &pitImage.cpuImage, *pitImage.stagedTexture), BRWL_CHAR_LITERAL("Failed to load the pitImage texture to the GPU.")))
                {   // we expect the function to clean up everything necessary
                    continue;
                };
            }
        }

        DR(uploadCommandList->Close());
        ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
        uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        for (int i = 0; i < countof(pitCollection.array); ++i)
        {
            if (uploading[i])
            {
                PitImage& pitImage = pitCollection.array[i];
                ++pitImage.uploadFenceValue;
                DR(uploadCommandQueue->Signal(pitImage.fence.Get(), pitImage.uploadFenceValue));
                // instead of waiting here, we check the completion state in render()
            }
        }
    }
}


void Visualization2Renderer::destroy(Renderer* r)
{

    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        PitImage& pitImage = pitCollection.array[i];

        if (pitImage.fence && pitImage.uploadFenceValue < pitImage.fence->GetCompletedValue())
        {
            DR(pitImage.fence->SetEventOnCompletion(pitImage.uploadFenceValue, pitImage.uploadEvent));
            WaitForSingleObject(uploadFenceEvent, INFINITE);
            ResetEvent(pitImage.uploadEvent);
        }
    }

    mainShader.destroy();

    initialized = false;
    volumeTexture.destroy();
    pitCollection.destroy();
    volumeTextureUploadFence = nullptr;
    volumeTextureFenceValue = 0;
    if (uploadFenceEvent)
    {
        CloseHandle(uploadFenceEvent);
        uploadFenceEvent = nullptr;
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

bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, TextureResource& texture)
{
    if (!dataSet->isValid() || !BRWL_VERIFY(texture.state == TextureResource::State::REQUESTING_UPLOAD, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))
    {
        return false;
    }

    texture.texture = nullptr;
    texture.uploadHeap = nullptr;
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
    texDesc.Format = DXGI_FORMAT_R16_SNORM;
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
        IID_PPV_ARGS(&texture.uploadHeap));
    

    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture upload heap.")))
    {
        texture.texture = nullptr;
        texture.uploadHeap = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    D3D12_SUBRESOURCE_DATA textureData {
        dataSet->getData(),
        dataSet->getStrideY(),
        dataSet->getStrideZ()
    };

    uint64_t result = UpdateSubresources(cmdList, texture.texture.Get(), texture.uploadHeap.Get(), 0, 0, 1, &textureData);
    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
    {
        texture.texture = nullptr;
        texture.uploadHeap = nullptr;
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

bool LoadFloatTexture2D(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Image* image, TextureResource& texture)
{
    if (!image->isValid() || !BRWL_VERIFY(texture.state == TextureResource::State::REQUESTING_UPLOAD, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))
    {
        return false;
    }

    texture.texture = nullptr;
    texture.uploadHeap = nullptr;
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
        IID_PPV_ARGS(&texture.uploadHeap));


    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture upload heap.")))
    {
        texture.texture = nullptr;
        texture.uploadHeap = nullptr;
        texture.state = TextureResource::State::FAILED;
        return false;
    }

    D3D12_SUBRESOURCE_DATA textureData {
        image->getData(),
        image->getStrideY(),
        image->getBufferSize()
    };

    uint64_t result = UpdateSubresources(cmdList, texture.texture.Get(), texture.uploadHeap.Get(), 0, 0, 1, &textureData);
    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
    {
        texture.texture = nullptr;
        texture.uploadHeap = nullptr;
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
