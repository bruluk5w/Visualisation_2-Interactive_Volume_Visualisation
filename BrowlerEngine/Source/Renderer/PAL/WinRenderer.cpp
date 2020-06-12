#include "PAL/WinRenderer.h"

#ifdef BRWL_PLATFORM_WINDOWS

#include "Common/Logger.h"

// DX12 additional headers
#include <DirectXMath.h>
#include "PAL/d3dx12.h"

#include "PAL/WinRendererParameters.h"

#ifdef BRWL_USE_DEAR_IM_GUI
#include "UI/ImGui/imgui.h"
#include "PAL/imgui_impl_dx12.h"
#include "PAL/imgui_impl_win32.h"
#include "UI/AppUi.h"
extern ImGuiContext* GImGui;
#endif // BRWL_USE_DEAR_IM_GUI

#include "AppRenderer.h"

namespace
{
    bool g_useSoftwareRasterizer = false;

    const BRWL_CHAR* GetDeviceRemovedReasonString(HRESULT reason){
        switch (reason)
        {
        case DXGI_ERROR_DEVICE_HUNG: return BRWL_CHAR_LITERAL("DEVICE HUNG"); 
        case DXGI_ERROR_DEVICE_REMOVED: return BRWL_CHAR_LITERAL("DEVICE REMOVED");
        case DXGI_ERROR_DEVICE_RESET: return BRWL_CHAR_LITERAL("DEVICE RESET");
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return BRWL_CHAR_LITERAL("DRIVER INTERNAL ERROR");
        case DXGI_ERROR_INVALID_CALL: return BRWL_CHAR_LITERAL("INVALID CALL");
        case S_OK: return BRWL_CHAR_LITERAL("Huh... actually everything should be fine.");
        default: return BRWL_CHAR_LITERAL("Unknown Reason.");
        }

        BRWL_UNREACHABLE();
    }
}

BRWL_RENDERER_NS


namespace PAL
{

    void HandleDeviceRemoved(HRESULT result, ID3D12Device* device, ::BRWL::Logger& logger)
    {
        if (BRWL_VERIFY(SUCCEEDED(result), nullptr))
            return;

        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
        {
            // TODO: test this https://docs.microsoft.com/en-us/windows/uwp/gaming/handling-device-lost-scenarios
            ComPtr<ID3D12DeviceRemovedExtendedData> dred;
            BRWL_EXCEPTION(SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&dred))), nullptr);

            D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT dredAutoBreadcrumbsOutput;
            D3D12_DRED_PAGE_FAULT_OUTPUT dredPageFaultOutput;
            BRWL_VERIFY(SUCCEEDED(dred->GetAutoBreadcrumbsOutput(&dredAutoBreadcrumbsOutput)), nullptr);
            BRWL_VERIFY(SUCCEEDED(dred->GetPageFaultAllocationOutput(&dredPageFaultOutput)), nullptr);


            const BRWL_CHAR* msg = nullptr;
            if (result == DXGI_ERROR_DEVICE_REMOVED) msg = BRWL_CHAR_LITERAL("DEVICE REMOVED!");
            if (result == DXGI_ERROR_DEVICE_RESET) msg = BRWL_CHAR_LITERAL("DEVICE RESET!");
            {
                ::BRWL::Logger::ScopedMultiLog m(&logger, ::BRWL::Logger::LogLevel::ERROR);
                logger.error(msg, &m);
                logger.error(GetDeviceRemovedReasonString(device->GetDeviceRemovedReason()), &m);
            }
        }
    }

    DXGI_FORMAT g_RenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    const D3D_FEATURE_LEVEL WinRenderer::featureLevel = D3D_FEATURE_LEVEL_11_0;
    const Vec4 WinRenderer::clearColor = Vec4(0.5, 0.5, 0.5, 1.0);


    WinRenderer::WinRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals) :
        BaseRenderer(eventSystem, globals),
        dxgiFactory(nullptr),
        dxgiAdapter(nullptr),
        device(nullptr),
        rtvHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
        srvHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE),
        frameContext{},
        frameIndex(0),
        commandQueue(nullptr),
        commandList(nullptr),
        frameFence(nullptr),
        frameFenceEvent(NULL),
        frameFenceLastValue(0),
        fontTextureDescriptorHandle{}
    {
#ifdef ENABLE_GRAPHICS_DEBUG_FEATURES
        ComPtr<ID3D12Debug> debugController0;
        if (BRWL_VERIFY(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController0))), BRWL_CHAR_LITERAL("Failed to enable D3D debug layer.")))
        {
            debugController0->EnableDebugLayer();
#ifdef _DEBUG
            ComPtr<ID3D12Debug1> debugController1;
            if (BRWL_VERIFY(SUCCEEDED(debugController0.As(&debugController1)), BRWL_CHAR_LITERAL("Failed to enable GPU-based validation.")))
            {
                debugController1->SetEnableGPUBasedValidation(true);
            }
#endif
            ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
            if (BRWL_VERIFY(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings))), BRWL_CHAR_LITERAL("Failed to enable D3D debug layer.")))
            {
                if (dredSettings != nullptr) {
                    dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }
            }
        }
#endif
    }

    WinRenderer::~WinRenderer()
    {
#ifdef ENABLE_GRAPHICS_DEBUG_FEATURES
        ComPtr<IDXGIDebug1> pDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));
        }
#endif
    }

    bool WinRenderer::init(const WinRendererParameters rendererParameters)
    {
#ifdef BRWL_USE_DEAR_IM_GUI
        IMGUI_CHECKVERSION();
        // Setup Dear ImGui context
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.Fonts->AddFontDefault();
#endif
        // Initialize Direct3D
        if (!createDevice(rendererParameters.hWnd, rendererParameters.initialDimensions.width, rendererParameters.initialDimensions.height))
        {
            destroyDevice();
            return false;;
        }

#ifdef BRWL_USE_DEAR_IM_GUI
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        ImGui_ImplWin32_Init(rendererParameters.hWnd);
#endif // BRWL_USE_DEAR_IM_GUI

        if (!BRWL_VERIFY(BaseRenderer::init(rendererParameters), BRWL_CHAR_LITERAL("Failed to init BaseRenderer")))
        {
            return false;
        }

#ifdef _DEBUG
        frameIdxChangeListenerHandle = eventSystem->registerListener(Event::FRAME_IDX_CHANGE, [this](Event, void* param) {return OnFrameIdxChange(castParam<Event::FRAME_IDX_CHANGE>(param)->newFrameIdx); });
#endif

        return true;
    }

#define DR(expression) HandleDeviceRemoved(expression, device.Get(), *logger)

    void WinRenderer::render()
    {
        if (!currentFramebufferHeight || !currentFramebufferWidth)
        {
            logger->info(BRWL_CHAR_LITERAL("Nothing to render, framebuffer too small."));
            return;
        }


        // Start new frame
#ifdef BRWL_USE_DEAR_IM_GUI
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
#endif // BRWL_USE_DEAR_IM_GUI

        // NOTIC: We open the command list early for the renderer method below to schedule resource barriers after some uploads have finished
        FrameContext* frameCtxt = waitForNextFrameResources();
        frameCtxt->CommandAllocator->Reset();
        DR(commandList->Reset(frameCtxt->CommandAllocator.Get(), nullptr));

        // Render
        BaseRenderer::render();

#ifdef BRWL_USE_DEAR_IM_GUI
        ImGui::Render();
#endif
    }

    void WinRenderer::draw()
    {
        if (!currentFramebufferHeight || !currentFramebufferWidth)
        {
            logger->info(BRWL_CHAR_LITERAL("Nothing to draw, framebuffer too small."));
            return;
        }

        // Draw

        UINT backBufferIdx = swapChain->GetCurrentBackBufferIndex();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = mainRenderTargetResource[backBufferIdx].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->ResourceBarrier(1, &barrier);
        commandList->ClearRenderTargetView(mainRenderTargetDescriptor[backBufferIdx].cpu, (float*)&clearColor, 0, nullptr);
        commandList->OMSetRenderTargets(1, &mainRenderTargetDescriptor[backBufferIdx].cpu, FALSE, nullptr);
        commandList->SetDescriptorHeaps(1, srvHeap.getPointerAddress());

        // ========= START DRAW SCENE ========= //

        // TODO: add option to toggle shader optimization during application run
        //SetBackgroundProcessingMode(
        //    D3D12_BACKGROUND_PROCESSING_MODE_ALLOW_INTRUSIVE_MEASUREMENTS,
        //    D3D_MEASUREMENTS_ACTION_KEEP_ALL,
        //    nullptr, nullptr);

        //// Here, prime the system by rendering some typical content.
        //// For example, a level flythrough.

        //SetBackgroundProcessingMode(
        //    D3D12_BACKGROUND_PROCESSING_MODE_ALLOWED,
        //    D3D12_MEASUREMENTS_ACTION_COMMIT_RESULTS,
        //    nullptr, nullptr);

#ifdef BRWL_USE_DEAR_IM_GUI
        BaseRenderer::draw();
        // Draw Gui last
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif // BRWL_USE_DEAR_IM_GUI

        // =========  END DRAW SCENE  ========= //
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &barrier);
        DR(commandList->Close());

        ID3D12CommandList* const ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(1, ppCommandLists);

        DR(swapChain->Present(1, 0)); // Present with vsync
        //DR(swapChain->Present(0, 0)); // Present without vsync

        UINT64 fenceValue = frameFenceLastValue + 1;
        DR(commandQueue->Signal(frameFence.Get(), fenceValue));
        frameFenceLastValue = fenceValue;
        getCurrentFrameContext()->FenceValue = fenceValue;


        if (!dxgiFactory->IsCurrent())
        {
            // TODO: FLUSH?
            logger->warning(BRWL_CHAR_LITERAL("Graphics adapters changed! Rescanning..."));
            destroyDevice();
            createDevice(params->hWnd, currentFramebufferWidth, currentFramebufferHeight);
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
        }
#undef DR
    }

    void WinRenderer::destroy(bool force /*= false*/)
    {
#ifdef _DEBUG
        eventSystem->unregisterListener(Event::FRAME_IDX_CHANGE, frameIdxChangeListenerHandle);
#endif
        waitForLastSubmittedFrame();

        BaseRenderer::destroy(force);
        
        destroyDevice();

        if (GImGui != NULL)
        {
            ImGui::DestroyContext();
        }

        ImGui_ImplWin32_Shutdown();
    }

#pragma region IMGUI COPY

    // Helper functions

    bool WinRenderer::createDevice(HWND hWnd, unsigned int framebufferWidth /*=0 */, unsigned int framebufferHeight /*=0 */)
    {
#ifndef __dxgi1_6_h__
#error Dude please get a Windows 10 and update, this was like 2018
#endif

        UINT flags = 0;
#if defined(_DEBUG)
        flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
        ComPtr<IDXGIFactory2> dxgiFactory2;

        if (!BRWL_VERIFY(CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory2)) == S_OK, BRWL_CHAR_LITERAL("Failed to create DXGIFactory2.")))
        {
            return false;
        }
        if (!BRWL_VERIFY(dxgiFactory2.As(&dxgiFactory) == S_OK, BRWL_CHAR_LITERAL("Failed to retrieve DXGIFactory6.")))
        {
            return false;
        }


        if (g_useSoftwareRasterizer)
        {
            ComPtr<IDXGIAdapter1> dxgiAdapter1;
            if (!BRWL_VERIFY(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)) == S_OK, BRWL_CHAR_LITERAL("Failed to get software rasterizer.")))
            {
                return false;
            }
            //if(!BRWL_VERIFY( == S_OK, BRWL_CHAR_LITERAL("")))
            if (!BRWL_VERIFY(dxgiAdapter1.As(&dxgiAdapter) == S_OK, BRWL_CHAR_LITERAL("Failed to get software rasterizer as DXGIAdapter4.")))
            {
                return false;
            }
        }
        else
        {
            ComPtr<IDXGIAdapter4> dxgiAdapter4;
            HRESULT enumerationResult;
            for (unsigned int i = 0; (enumerationResult = dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_MINIMUM_POWER, IID_PPV_ARGS(&dxgiAdapter4))) == S_OK; ++i)
            {
                DXGI_ADAPTER_DESC1 desc;
                if (!BRWL_VERIFY(SUCCEEDED(dxgiAdapter4->GetDesc1(&desc)), BRWL_CHAR_LITERAL("Failed to get adapter description.")))
                {
                    return false;
                }

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

                // Create device and keep it if it supports the requested feature level
                if (BRWL_VERIFY(SUCCEEDED(D3D12CreateDevice(dxgiAdapter4.Get(), featureLevel, IID_PPV_ARGS(&device))), BRWL_CHAR_LITERAL("Failed to create D3D12Device.")))
                {
                    Logger::LogLevel logLvl = Logger::LogLevel::INFO;
                    // check root signature version 1_1 support
                    D3D12_FEATURE_DATA_ROOT_SIGNATURE highestVersion = { D3D_ROOT_SIGNATURE_VERSION_1_1 };
                    const BRWL_CHAR* errorMsg = nullptr;
                    if (!SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &highestVersion, sizeof(highestVersion))))
                    {
                        logLvl = Logger::LogLevel::WARNING;
                        errorMsg = BRWL_CHAR_LITERAL("Rejected: No support for DX12 root signature version 1_1. Please update driver.");
                    }

                    Logger::ScopedMultiLog ml(logger.get(), logLvl);
                    dxgiAdapter = dxgiAdapter4;
                    BRWL_CHAR buff[256] = { 0 };
                    BRWL_SNPRINTF(buff, countof(buff), BRWL_CHAR_LITERAL("Direct3D12 Adapter (%u): VendorId: %04X, DeviceId: %04X, Description: %ls"), i, desc.VendorId, desc.DeviceId, desc.Description);
                    logger->log(buff, logLvl, &ml);
                    logger->log(BRWL_NEWLINE, logLvl, &ml);
                    logger->log(errorMsg, logLvl, &ml);
                    if (errorMsg == nullptr)
                    {
                        BRWL_SNPRINTF(buff, countof(buff), BRWL_CHAR_LITERAL("Dedicated Video Memory: %zuMB, Dedicated System Memory: %zuMB, Shared System Memory: %zuMB"), desc.DedicatedVideoMemory / 1048576, desc.DedicatedSystemMemory / 1048576, desc.SharedSystemMemory / 1048576);
                        logger->log(buff, logLvl, &ml);
                        break;
                    }
                }

                // continue trying the others, in case someone has an old potato discrete GPU but a shiny new CPU with integrated graphics, then, well, let's go with that...
            }

            const BRWL_CHAR* errorMsg = BRWL_CHAR_LITERAL("Could not find a graphics adapter which supports all the requirements.");

            if (!BRWL_VERIFY(enumerationResult == S_OK, errorMsg))
            {
                logger->error(errorMsg);
                return false;
            }

            D3D12_FEATURE_DATA_EXISTING_HEAPS dredSupport;
            if (!SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS, &dredSupport, sizeof(dredSupport))) || !dredSupport.Supported)
            {
                logger->warning(BRWL_CHAR_LITERAL("Device Removed Extended Data not supported on this device. Good luck!"));
            }

        }

#if defined(_DEBUG)
        // Enable debug messages in debug mode.
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (BRWL_VERIFY(SUCCEEDED(device.As(&pInfoQueue)), BRWL_CHAR_LITERAL("Failed to get debug info queue.")))
        {
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
            // Suppress whole categories of messages
            //D3D12_MESSAGE_CATEGORY Categories[] = {};

            // Suppress messages based on their severity level
            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            // Suppress individual messages by their ID
            D3D12_MESSAGE_ID DenyIds[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            //NewFilter.DenyList.NumCategories = _countof(Categories);
            //NewFilter.DenyList.pCategoryList = Categories;
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;
            NewFilter.DenyList.NumIDs = _countof(DenyIds);
            NewFilter.DenyList.pIDList = DenyIds;

            if (!BRWL_VERIFY(SUCCEEDED(pInfoQueue->PushStorageFilter(&NewFilter)), BRWL_CHAR_LITERAL("Failed to set debug info queue filter.")))
            {
                return false;
            }
        }
#endif // defined(_DEBUG)

        
        if (!rtvHeap.create(device.Get(), numBackBuffers))
        {
            return false;
        }

        for (UINT i = 0; i < numBackBuffers; i++)
        {
            mainRenderTargetDescriptor[i] = rtvHeap.allocateHandle(
#ifdef _DEBUG
                BRWL_CHAR_LITERAL("MainRenderTarget")
#endif
            );
        }
        
        if (!srvHeap.create(device.Get(), 300))
        {
            return false;
        }

        {
            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 0;
            if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue))), BRWL_CHAR_LITERAL("Failed to create direct command queue.")))
            {
                return false;
            }
        }

        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
            if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frameContext[i].CommandAllocator))), BRWL_CHAR_LITERAL("Failed to create direct command allocator.")))
            {
                return false;
            }            
        }
        
        if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameContext[0].CommandAllocator.Get(), NULL, IID_PPV_ARGS(&commandList))), BRWL_CHAR_LITERAL("Failed to create command list.")))
        {
            return false;
        }

        if (!BRWL_VERIFY(SUCCEEDED(commandList->Close()), BRWL_CHAR_LITERAL("Failed to close command list.")))
        {
            return false;
        }

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&frameFence))), BRWL_CHAR_LITERAL("Failed to create frame fence object.")))
        {
            return false;
        }

        frameFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!BRWL_VERIFY(frameFenceEvent != NULL, BRWL_CHAR_LITERAL("Failed to create frame fence event.")))
        {
            return false;
        }

        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC1 sd;
        {
            memset(&sd, 0, sizeof(sd));
            sd.BufferCount = numBackBuffers;
            sd.Width = framebufferWidth;
            sd.Height = framebufferHeight;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            sd.Scaling = DXGI_SCALING_STRETCH;
            sd.Stereo = FALSE;
        }

        ComPtr<IDXGISwapChain1> swapChain1;
        if( dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &sd, NULL, NULL, &swapChain1) != S_OK ||
            swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain)) != S_OK)
            return false;
        swapChain->SetMaximumFrameLatency(numBackBuffers);
        swapChainWaitableObject = swapChain->GetFrameLatencyWaitableObject();
        
        createRenderTargets();
        currentFramebufferWidth = framebufferWidth;
        currentFramebufferHeight = framebufferHeight;

        fontTextureDescriptorHandle = srvHeap.allocateHandle(
#ifdef _DEBUG
            BRWL_CHAR_LITERAL("FontTextureDescriptor")
#endif
        );
        ImGui_ImplDX12_Init(device, NUM_FRAMES_IN_FLIGHT, g_RenderTargetFormat, /*since this is unused, we currently also don't pass any resource*/{ },
            fontTextureDescriptorHandle.cpu, fontTextureDescriptorHandle.gpu);

        if (appRenderer && !appRenderer->isInitalized() && !BRWL_VERIFY(appRenderer->rendererInit(this), BRWL_CHAR_LITERAL("Failed to initialize the app renderer.")))
        {
            return false;
        }

        return true;
    }

    void WinRenderer::destroyDevice()
    {
        ImGui_ImplDX12_Shutdown();
        fontTextureDescriptorHandle.destroy();

        if (appRenderer)
        {
            appRenderer->rendererDestroy(static_cast<Renderer*>(this));
        }

        destroyRenderTargets();
        for (UINT i = 0; i < numBackBuffers; i++)
        {
            mainRenderTargetDescriptor[i].destroy();
        }

        swapChain = nullptr;
        if (swapChainWaitableObject != NULL)
        {
            CloseHandle(swapChainWaitableObject);
            swapChainWaitableObject = NULL;
        }

        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        {
            frameContext[i].CommandAllocator = nullptr;
            frameContext[i].FenceValue = 0;
        }

        commandQueue = nullptr;
        commandList = nullptr;
        rtvHeap.destroy();
        srvHeap.destroy();
        frameFence = nullptr;
        frameFenceLastValue = 0;
        if (frameFenceEvent)
        {
            CloseHandle(frameFenceEvent);
            frameFenceEvent = nullptr;
        }

        device = nullptr;
        dxgiAdapter = nullptr;
        dxgiFactory = nullptr;
    }

    void WinRenderer::createRenderTargets()
    {
        for (UINT i = 0; i < numBackBuffers; i++)
        {
            ComPtr<ID3D12Resource> pBackBuffer = nullptr;
            swapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            device->CreateRenderTargetView(pBackBuffer.Get(), NULL, mainRenderTargetDescriptor[i].cpu);
            mainRenderTargetResource[i] = pBackBuffer;
            mainRenderTargetResource[i]->SetName(BRWL_CHAR_LITERAL("RenderTarget"));
        }
    }

    void WinRenderer::destroyRenderTargets()
    {
        waitForLastSubmittedFrame();

        for (UINT i = 0; i < numBackBuffers; i++)
        {
            mainRenderTargetResource[i] = nullptr;
        }
    }

    void WinRenderer::waitForLastSubmittedFrame()
    {
        FrameContext* frameCtxt = &frameContext[frameIndex % NUM_FRAMES_IN_FLIGHT];

        UINT64 fenceValue = frameCtxt->FenceValue;
        if (fenceValue == 0)
            return; // No fence was signaled

        frameCtxt->FenceValue = 0;
        if (frameFence->GetCompletedValue() >= fenceValue)
            return;

        frameFence->SetEventOnCompletion(fenceValue, frameFenceEvent);
        WaitForSingleObject(frameFenceEvent, INFINITE);
    }

    WinRenderer::FrameContext* WinRenderer::waitForNextFrameResources()
    {
        ++frameIndex;

        HANDLE waitableObjects[] = { swapChainWaitableObject, NULL };
        DWORD numWaitableObjects = 1;

        FrameContext* frameCtxt = getCurrentFrameContext();
        UINT64 fenceValue = frameCtxt->FenceValue;
        if (fenceValue != 0) // means no fence was signaled
        {
            frameCtxt->FenceValue = 0;
            frameFence->SetEventOnCompletion(fenceValue, frameFenceEvent);
            waitableObjects[1] = frameFenceEvent;
            numWaitableObjects = 2;
        }

        WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

        return frameCtxt;
    }

    WinRenderer::FrameContext* WinRenderer::getCurrentFrameContext()
    {
        return &frameContext[frameIndex % NUM_FRAMES_IN_FLIGHT];
    }

    void WinRenderer::resizeSwapChain(HWND hWnd, int width, int height)
    {
        BRWL_EXCEPTION(width > 0 && height > 0, BRWL_CHAR_LITERAL("Invalid dimensions."));
        // Flush the GPU queue to make sure the swap chain's back buffers
        // are not being referenced by an in-flight command list.
        //Flush(commandQueue, g_Fence, g_FenceValue, g_FenceEvent);

        if (!dxgiFactory->IsCurrent())
        {
            // TODO: FLUSH?
            logger->warning(BRWL_CHAR_LITERAL("Graphics adapters changed! Rescanning..."));
            destroyDevice();
            createDevice(params->hWnd, currentFramebufferWidth, currentFramebufferHeight);
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            return;
        }

        destroyRenderTargets();

        DXGI_SWAP_CHAIN_DESC1 sd;
        HRESULT success = swapChain->GetDesc1(&sd);
        BRWL_EXCEPTION(SUCCEEDED(success), BRWL_CHAR_LITERAL("Failed to retrieve swap chain description."));
        success = swapChain->ResizeBuffers(sd.BufferCount, width, height, sd.Format, sd.Flags);
        BRWL_EXCEPTION(SUCCEEDED(success), BRWL_CHAR_LITERAL("Failed to resize swap chain."));
        createRenderTargets();

        //sd.Width = width;
        //sd.Height = height;
        //swapChain->Release();

        //CloseHandle(swapChainWaitableObject);

        //ComPtr<IDXGISwapChain1> swapChain1;
        //dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &sd, NULL, NULL, &swapChain1);
        //swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
        //swapChain1->Release();
        //dxgiFactory->Release();

        //swapChain->SetMaximumFrameLatency(numBackBuffers);

        //swapChainWaitableObject = swapChain->GetFrameLatencyWaitableObject();
        assert(swapChainWaitableObject != NULL);
    }

    void WinRenderer::OnFramebufferResize()
    {
        currentFramebufferWidth = Utils::max<unsigned int>(currentFramebufferWidth, 1);
        currentFramebufferHeight = Utils::max<unsigned int>(currentFramebufferHeight, 1);

        waitForLastSubmittedFrame();
        ImGui_ImplDX12_InvalidateDeviceObjects();
        resizeSwapChain(params->hWnd, currentFramebufferWidth, currentFramebufferHeight);
        ImGui_ImplDX12_CreateDeviceObjects();
        preRender();
        render();
        draw();
    }

#pragma endregion //IMGUI COPY

} // namespace PAL


BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS

