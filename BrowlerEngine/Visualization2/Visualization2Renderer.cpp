#include "Visualization2Renderer.h"

#include "Common/Logger.h"
#include "Common/BoundingBox.h"
#include "Core/BrowlerEngine.h"
#include "Preintegration.h"
#include "Renderer/Renderer.h"
#include "Renderer/PAL/imgui_impl_dx12.h"
#include "Core/Events.h"

BRWL_RENDERER_NS


Visualization2Renderer::Visualization2Renderer() :
    AppRenderer(),
    uiResultIdx(0),
    uiResults{ {},{} },
    fonts{ 0 },
    dataSetHandle(TextureHandle::Invalid),
    pitCollection(),
    assetPathMutex(),
    assetPath(BRWL_CHAR_LITERAL("./Assets/DataSets/stagbeetle832x832x494.dat")),
    mainShader(),
    initialized(false),
    skipFrame(false),
    hasViewChanged(true),
    hasCameraMovedListenerHandle(0)
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
    
    D3D12_FEATURE_DATA_D3D12_OPTIONS FeatureData;
    ZeroMemory(&FeatureData, sizeof(FeatureData));
    if (!BRWL_VERIFY(SUCCEEDED(r->getDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData))), BRWL_CHAR_LITERAL("Failed to get device freature data.")))
    {
        return false;        
    }

    if (!BRWL_VERIFY(FeatureData.TypedUAVLoadAdditionalFormats, BRWL_CHAR_LITERAL("Uav load of type R32G32B32A32_FLOAT not supported. Sorry, try another GPU :(.")))
    {
        return false;
    }

    LoadFonts(uiResults[uiResultIdx].settings.fontSize);

    if (!BRWL_VERIFY(ReloadVolumeAsset(r), BRWL_CHAR_LITERAL("Failed to load volume.")))
    {
        return false;
    }
    
    pitCollection.create(r->getTextureManager());

    if (!BRWL_VERIFY(mainShader.create(r), BRWL_CHAR_LITERAL("Failed to create main shader.")))
    {
        return false;
    }

    hasCameraMovedListenerHandle = r->getEventSystem()->registerListener(Event::CAM_HAS_MOVED, [this](Event, void*) { hasViewChanged = true; return true; });

    hasViewChanged = initialized = true;

    return initialized;
}

bool Visualization2Renderer::ReloadVolumeAsset(BRWL::Renderer::Renderer* r)
{
    DataSetS16* dataSet = dynamic_cast<DataSetS16*>(&*dataSetHandle);
    bool needsRefresh = dataSet->isValid();
    BRWL_STR file;
    {
        std::scoped_lock(assetPathMutex);
        file = assetPath.c_str();
    }

    needsRefresh |= BRWL_STRCMP(file.c_str(), dataSet->getSourcePath()) != 0;

    if (needsRefresh)
    {
        {
            Logger::ScopedMultiLog ml(engine->logger.get(), Logger::LogLevel::INFO);
            engine->logger->info(BRWL_CHAR_LITERAL("Loading asset from disk: "), &ml);
            engine->logger->info(file.c_str(), &ml);
        }

        dataSet->loadFromFile(file.c_str());
        if (!BRWL_VERIFY(dataSet->isValid(), BRWL_CHAR_LITERAL("Failed to load dataset.")))
        {
            return false;
        }

        if (!dataSetHandle.startLoad())
        {
            return false;
        }
    }

    return true;
}

void Visualization2Renderer::preRender(Renderer* renderer)
{
    const UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    if (v.settings.fontSize != r.settings.fontSize) {
        renderer->waitForLastSubmittedFrame();
        LoadFonts(r.settings.fontSize);
        v.settings.fontSize = r.settings.fontSize;
    }
}


void Visualization2Renderer::render(Renderer* renderer)
{
    if (!initialized) return;

#ifdef BRWL_USE_DEAR_IM_GUI
    if (!g_FontDescHandle->isResident())
    {
        skipFrame = true;
        return;
    }
    else
    {
        ::ImGui::GetIO().Fonts->TexID = (ImTextureID)g_FontDescHandle->getResident().residentGpu.ptr;
    }
#endif

    if (!ReloadVolumeAsset(renderer))
    {
        renderer->log(BRWL_CHAR_LITERAL("Failed to load volume data!"), Logger::LogLevel::Error);
    }

    //-------------------
    // swap pit images
    //--------------------
    
    //for (int i = 0; i < countof(pitCollection.array); ++i)
    //{
    //    PitImage& pitImage = pitCollection.array[i];
    //    const uint64_t completed = pitImage.fence->GetCompletedValue();

    //    // swap texture if ready
    //    if (pitImage.stagedTexture->state == TextureResource::State::LOADING && completed >= pitImage.uploadFenceValue)
    //    {
    //        pitImage.stagedTexture->state = TextureResource::State::RESIDENT;
    //        pitImage.stagedTexture->uploadHeap = nullptr; // free some resources
    //        // We might still have some frames in flight with the old texture
    //        // We have to wait until we are sure we that are the only one tampering with resources
    //        renderer->waitForLastSubmittedFrame();
    //        std::swap(pitImage.stagedTexture, pitImage.liveTexture);
    //        // release old texture
    //        pitImage.stagedTexture->destroy();
    //        // Indicate new resource to be used by a compute shader.
    //        renderer->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pitImage.liveTexture->texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
    //        hasViewChanged = true;
    //    }
    //}

    //---------------------
    // UI
    //----------------------

    // Swap objects for communication with front-end
    uiResultIdx = uiResultIdx ? 0 : 1;
    UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values

    // set textures to the front-end
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        const PitImage& pitImage = pitCollection.array[i];
        v.transferFunctions.array[i].textureID = pitImage.liveTexture->state != TextureResource::State::RESIDENT ? nullptr : (ImTextureID)pitImage.liveTexture->descriptorHandle->getResident().residentGpu.ptr;
    }

    v.remainingSlices = mainShader.getNumRemainingSlices();
    r.settings.vsync = renderer->getVSync();

    // execute UI and retrieve results
    ImGui::PushFont(fonts[ENUM_CLASS_TO_NUM(r.settings.font)]);
    renderAppUI(r, v);
    ImGui::PopFont();

    ImGui::Render();

    //-------------------
    // recompute PIT images
    // (wait for ongoing upload to finish +) request upload
    //-------------------

    // Check if textures should be recomputed
    bool mustRecompute[countof(*((decltype(pitCollection.array)*)nullptr))] = { false };
    bool blocked[countof(mustRecompute)] = { false };
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        PitImage& pitImage = pitCollection.array[i];
        UIResult::TransferFunction& tFuncValue = v.transferFunctions.array[i];
        UIResult::TransferFunction& tFuncResult = r.transferFunctions.array[i];
        if (!pitImage.cpuImage.isValid() || tFuncValue.bitDepth != tFuncResult.bitDepth ||
            memcmp(tFuncValue.transferFunction, tFuncResult.transferFunction, tFuncValue.getArrayLength() * sizeof(UIResult::TransferFunction::sampleT)) != 0)
        {
            mustRecompute[i] = true;
            // If staged texture is not free, wait for upload to complete
            if (pitImage.stagedTexture->state != TextureResource::State::NONE)
            {
                blocked[i] = true;
                // if the staged resource is currently upoading then the fence value has do be lower than the one which we remembered
                // but only do a weak check since it could theoretically finish in this very moment
                uint64_t completedValue = pitImage.fence->GetCompletedValue();
                BRWL_CHECK(completedValue <= pitImage.uploadFenceValue, BRWL_CHAR_LITERAL("Invalid fence/staged resource state."));
                uint64_t x = pitImage.fence->GetCompletedValue();
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
    unsigned int numWaitHandles = 0;
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        if (blocked[i])
        {
            PitImage& pitImage = pitCollection.array[i];
            waitHandles[numWaitHandles] = pitImage.uploadEvent;
            ++numWaitHandles;
        }
    }

    // wait and clean up event state 
    if (numWaitHandles)
    {
        WaitForMultipleObjects(numWaitHandles, waitHandles, true, INFINITE);
    }

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

    if (v.settings.freeCamMovement != r.settings.freeCamMovement)
    {
        BoolParam param{ r.settings.freeCamMovement };
        engine->eventSystem->postEvent<::BRWL::Event::SET_FREE_CAM_MOVEMENT>(&param);
        v.settings.freeCamMovement = r.settings.freeCamMovement;
        hasViewChanged = true;
    }

    if (v.settings.voxelsPerCm != r.settings.voxelsPerCm) {
        hasViewChanged = true;
        v.settings.voxelsPerCm = r.settings.voxelsPerCm;
    }
    if (v.settings.numSlicesPerVoxel != r.settings.numSlicesPerVoxel) {
        hasViewChanged = true;
        v.settings.numSlicesPerVoxel = r.settings.numSlicesPerVoxel;
    }
    if (v.settings.drawOrthographicXRay != r.settings.drawOrthographicXRay) {
        hasViewChanged = true;
        v.settings.drawOrthographicXRay = r.settings.drawOrthographicXRay;
    }
    if (v.light != r.light) {
        hasViewChanged = true;
        v.light = r.light;
    }
    
    v.settings.font = r.settings.font;
    v.settings.drawAssetBoundaries = r.settings.drawAssetBoundaries;
    v.settings.drawViewingVolumeBoundaries = r.settings.drawViewingVolumeBoundaries;
    mainShader.render();
}

#define DR(expression) PAL::HandleDeviceRemoved(expression, r->device.Get(), r->logger.get())

// todo: instead of passing the whole renderer, pass a context with the necessary pointers (evtl. also pointer to renderer)
// but then remove public getters for srvHeap, command list etc.
void Visualization2Renderer::draw(Renderer* r)
{
    if (!initialized || skipFrame)
    {
        skipFrame = false;
        return;
    }

    PAL::DescriptorHeap& srvHeap = r->getSrvHeap();

    if (!BRWL_VERIFY(srvHeap.isCreated() && srvHeap.getType() == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, BRWL_CHAR_LITERAL("Invalid descriptor heap.")))
    {
        return;
    }

    // If we have all resources then we draw else we wait another frame
    if (pitCollection.isResident() && dataSetHandle.isResident())
    {
        const MainShader::DrawData drawData {
            &dataSetHandle,
            &pitCollection,
            uiResults[0].settings.voxelsPerCm,
            uiResults[0].settings.numSlicesPerVoxel,
            uiResults[0].settings.drawAssetBoundaries,
            uiResults[0].settings.drawViewingVolumeBoundaries,
            uiResults[0].settings.drawOrthographicXRay,
            hasViewChanged,
            {
                MainShader::DrawData::Light::Type::DIRECTIONAL,
                uiResults[0].light.coords,
                uiResults[0].light.color
            }
        };
        hasViewChanged = false;
        mainShader.draw(r->getDevice(), r->getCommandList(), drawData);
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
                pitImage.stagedTexture->descriptorHandle = r->srvHeap.allocateOne(BRWL_CHAR_LITERAL("StagedPitTexture"));
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

#undef DR

void Visualization2Renderer::destroy(Renderer* r)
{
    if (r->getEventSystem()) {
        r->getEventSystem()->unregisterListener(Event::CAM_HAS_MOVED, hasCameraMovedListenerHandle);
    }

    mainShader.destroy();

    initialized = false;
    dataSetHandle.destroy();
    pitCollection.destroy();
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
//
//bool LoadVolumeTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const DataSet* dataSet, TextureResource& texture)
//{
//    if (!dataSet->isValid() || !BRWL_VERIFY(texture.state == TextureResource::State::REQUESTING_UPLOAD, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))
//    {
//        return false;
//    }
//
//    texture.texture = nullptr;
//    texture.uploadHeap = nullptr;
//    // =========================================
//    // TODO: add a second path for 8bit textures
//    // =========================================
//    D3D12_RESOURCE_DESC texDesc;
//    memset(&texDesc, 0, sizeof(texDesc));
//    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
//    texDesc.Alignment = 0;
//    texDesc.Width = dataSet->getSizeX();
//    texDesc.Height = dataSet->getSizeY();
//    texDesc.DepthOrArraySize= dataSet->getSizeZ();
//    texDesc.MipLevels = 1;
//    texDesc.Format = DXGI_FORMAT_R16_SNORM;
//    texDesc.SampleDesc = { 1,0 };
//    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//    HRESULT hr = device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//        D3D12_HEAP_FLAG_NONE,
//        &texDesc,
//        D3D12_RESOURCE_STATE_COMMON,
//        nullptr,
//        IID_PPV_ARGS(&texture.texture));
//
//    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture.")))
//    {
//        texture.texture = nullptr;
//        texture.state = TextureResource::State::FAILED;
//        return false;
//    }
//
//    texture.texture->SetName(dataSet->getSourcePath());
//
//    uint64_t requiredSize = GetRequiredIntermediateSize(texture.texture.Get(), 0, 1);
//    BRWL_CHAR buf[100];
//    BRWL_SNPRINTF(buf, countof(buf), BRWL_CHAR_LITERAL("Loading volume data. Required VRAM: %.2fMB"), (float)requiredSize / 1048576.f);
//    engine->logger->info(buf);
//    // Create the GPU upload buffer.
//    hr = device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(&texture.uploadHeap));
//    
//
//    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture upload heap.")))
//    {
//        texture.texture = nullptr;
//        texture.uploadHeap = nullptr;
//        texture.state = TextureResource::State::FAILED;
//        return false;
//    }
//
//    D3D12_SUBRESOURCE_DATA textureData {
//        dataSet->getData(),
//        dataSet->getStrideY(),
//        dataSet->getStrideZ()
//    };
//
//    uint64_t result = UpdateSubresources(cmdList, texture.texture.Get(), texture.uploadHeap.Get(), 0, 0, 1, &textureData);
//    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
//    {
//        texture.texture = nullptr;
//        texture.uploadHeap = nullptr;
//        texture.state = TextureResource::State::FAILED;
//        return false;
//    }
//
//    // Create a SRV for the texture
//    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//    srvDesc.Format = texDesc.Format;
//    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
//    srvDesc.Texture3D.MipLevels = 1;
//    device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, texture.descriptorHandle->getNonResident().cpu);
// 
//    texture.state = TextureResource::State::LOADING;
//    return true;
//}

//bool LoadFloatTexture2D(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const Image* image, TextureResource& texture)
//{
//    if (!image->isValid() || !BRWL_VERIFY(texture.state == TextureResource::State::REQUESTING_UPLOAD, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))
//    {
//        return false;
//    }
//
//    texture.texture = nullptr;
//    texture.uploadHeap = nullptr;
//    D3D12_RESOURCE_DESC texDesc;
//    memset(&texDesc, 0, sizeof(texDesc));
//    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//    texDesc.Alignment = 0;
//    texDesc.Width = image->getSizeX();
//    texDesc.Height = image->getSizeY();
//    texDesc.DepthOrArraySize = 1;
//    texDesc.MipLevels = 1;
//    texDesc.Format = DXGI_FORMAT_R32_FLOAT;
//    texDesc.SampleDesc = { 1, 0 };
//    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // todo: try swizzled texture, may be faster
//    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//    HRESULT hr = device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//        D3D12_HEAP_FLAG_NONE,
//        &texDesc,
//        D3D12_RESOURCE_STATE_COMMON,
//        nullptr,
//        IID_PPV_ARGS(&texture.texture));
//
//    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture.")))
//    {
//        texture.texture = nullptr;
//        texture.state = TextureResource::State::FAILED;
//        return false;
//    }
//
//    texture.texture->SetName(image->getName());
//
//    uint64_t requiredSize = GetRequiredIntermediateSize(texture.texture.Get(), 0, 1);
//    // Create the GPU upload buffer.
//    hr = device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(requiredSize),
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(&texture.uploadHeap));
//
//
//    if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture upload heap.")))
//    {
//        texture.texture = nullptr;
//        texture.uploadHeap = nullptr;
//        texture.state = TextureResource::State::FAILED;
//        return false;
//    }
//
//    D3D12_SUBRESOURCE_DATA textureData {
//        image->getData(),
//        image->getStrideY(),
//        image->getBufferSize()
//    };
//
//    uint64_t result = UpdateSubresources(cmdList, texture.texture.Get(), texture.uploadHeap.Get(), 0, 0, 1, &textureData);
//    if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload volume texture.")))
//    {
//        texture.texture = nullptr;
//        texture.uploadHeap = nullptr;
//        texture.state = TextureResource::State::FAILED;
//        return false;
//    }
//
//    // Create a SRV for the texture
//    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//    srvDesc.Format = texDesc.Format;
//    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//    srvDesc.Texture2D.MipLevels = 1;
//    srvDesc.Texture2D.PlaneSlice = 0;
//    device->CreateShaderResourceView(texture.texture.Get(), &srvDesc, texture.descriptorHandle->getNonResident().cpu);
//
//    texture.state = TextureResource::State::LOADING;
//    return true;
//}



BRWL_RENDERER_NS_END
