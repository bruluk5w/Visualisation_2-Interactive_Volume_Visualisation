#include "Visualization2Renderer.h"

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
    dataSetHandle(BaseTextureHandle::Invalid),
    pitCollection(),
    assetPathMutex(),
    assetPath(BRWL_CHAR_LITERAL("./Assets/DataSets/stagbeetle832x832x494.dat")),
    mainShader(),
    initialized(false),
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

    hasCameraMovedListenerHandle = r->getEventSystem()->registerListener(Event::CAM_HAS_MOVED, [this](Event, void*) { 
        hasViewChanged = true; return true; });

    hasViewChanged = initialized = true;

    return initialized;
}

bool Visualization2Renderer::ReloadVolumeAsset(BRWL::Renderer::Renderer* r)
{
    if (dataSetHandle == BaseTextureHandle::Invalid)
    {
        dataSetHandle = r->getTextureManager()->createTexture<DataSetS16>(BRWL_CHAR_LITERAL("Volume Data Set"));
    }

    DataSetS16* dataSet = dynamic_cast<DataSetS16*>(&*dataSetHandle);
    bool needsRefresh = !dataSet->isValid();
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
    // reload font textures if font size setting changed
    const UIResult& r = uiResults[uiResultIdx]; // results
    UIResult& v = uiResults[uiResultIdx ? 0 : 1]; // values
    if (v.settings.fontSize != r.settings.fontSize)
    {
        PAL::ResourceFrame f(&renderer->getSrvHeap());
        renderer->waitForLastSubmittedFrame();
        LoadFonts(r.settings.fontSize);
        v.settings.fontSize = r.settings.fontSize;
    }
}


void Visualization2Renderer::render(Renderer* renderer)
{
    if (!initialized) return;

    if (!ReloadVolumeAsset(renderer))
    {
        renderer->log(BRWL_CHAR_LITERAL("Failed to load volume data!"), Logger::LogLevel::ERROR);
    }

    // todo: remove this if not needed anymore
    hasViewChanged |= renderer->anyTextureBecameResident;

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
        const BaseTextureHandle& pitImage = pitCollection.array[i];
        v.transferFunctions.array[i].textureID = pitImage.isResident() ? (ImTextureID)pitImage.asPlatformHandle()->getDescriptorHandle()->getResident().residentGpu.ptr : nullptr;
    }

    v.remainingSlices = mainShader.getNumRemainingSlices();
    r.settings.vsync = renderer->getVSync();

    // execute UI and retrieve results
    ImGui::PushFont(fonts[ENUM_CLASS_TO_NUM(r.settings.font)]);
    renderAppUI(r, v);
    ImGui::PopFont();

    //-------------------
    // recompute PIT images
    // (wait for ongoing upload to finish +) request upload
    //-------------------

    // Check if textures should be recomputed
    bool mustRecompute[countof(*((decltype(pitCollection.array)*)nullptr))] = { false };
    bool anyRecompute = false;
    for (int i = 0; i < countof(pitCollection.array); ++i)
    {
        BaseTextureHandle& pitImage = pitCollection.array[i];
        UIResult::TransferFunction& tFuncValue = v.transferFunctions.array[i];
        UIResult::TransferFunction& tFuncResult = r.transferFunctions.array[i];
        if (!pitImage->isValid() || tFuncValue.bitDepth != tFuncResult.bitDepth ||
            memcmp(tFuncValue.transferFunction, tFuncResult.transferFunction, tFuncValue.getArrayLength() * sizeof(UIResult::TransferFunction::sampleT)) != 0)
        {
            mustRecompute[i] = anyRecompute = true;
        }
    }

    if (anyRecompute)
    {
        renderer->getTextureManager()->waitForPendingUploads(pitCollection.array, countof(pitCollection.array));

        // Recompute
        for (int i = 0; i < countof(pitCollection.array); ++i)
        {
            UIResult::TransferFunction& tFuncValue = v.transferFunctions.array[i];
            UIResult::TransferFunction& tFuncResult = r.transferFunctions.array[i];
            if (mustRecompute[i])
            {
                // Recompute texture
                const int tableSize = tFuncResult.getArrayLength();
                BaseTextureHandle& pitImage = pitCollection.array[i];
                if (!pitImage->isValid() || pitImage->getSizeX() != tableSize)
                    pitImage->create(tableSize, tableSize);
                else
                    pitImage->zero();

                makePreintegrationTable<>(*dynamic_cast<PitCollection::PitImage*>(&*pitImage), tFuncResult.transferFunction, tFuncResult.getArrayLength());

                pitImage.startLoad();
            }

            // set front-end request satisfied
            // todo: remove copy and maybe use only one instance of transfer function struct
            tFuncValue = tFuncResult;
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
    mainShader.render(renderer->getCommandList());
}

#define DR(expression) PAL::HandleDeviceRemoved(expression, r->device.Get(), r->logger.get())

// todo: instead of passing the whole renderer, pass a context with the necessary pointers (evtl. also pointer to renderer)
// but then remove public getters for srvHeap, command list etc.
void Visualization2Renderer::draw(Renderer* r)
{
    if (!initialized)
    {
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
        MainShader::DrawData drawData {
            *dataSetHandle.asPlatformHandle(),
            pitCollection,
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
}

#undef DR

void Visualization2Renderer::destroy(Renderer* r)
{
    if (r->getEventSystem()) {
        r->getEventSystem()->unregisterListener(Event::CAM_HAS_MOVED, hasCameraMovedListenerHandle);
    }

    mainShader.destroy();

    initialized = false;
    {
        PAL::ResourceFrame f(&r->getSrvHeap());
        dataSetHandle.destroy();
        pitCollection.destroy();
    }
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


BRWL_RENDERER_NS_END
