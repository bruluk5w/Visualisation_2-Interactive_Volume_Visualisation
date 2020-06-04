#include "PAL/WinRenderer.h"

#ifdef BRWL_PLATFORM_WINDOWS

#include "Common/Logger.h"

// DX12 additional headers
#include <DirectXMath.h>
#include <PAL/d3dx12.h>

#include "PAL/WinRendererParameters.h"

#ifdef BRWL_USE_DEAR_IM_GUI
#include "UI/ImGui/imgui.h"
#include "PAL/imgui_impl_dx12.h"
#include "PAL/imgui_impl_win32.h"
#include "UI/TestUi.h"
extern ImGuiContext* GImGui;
#endif // BRWL_USE_DEAR_IM_GUI

#include "Common/BrwlMath.h"

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

    const D3D_FEATURE_LEVEL WinRenderer::featureLevel = D3D_FEATURE_LEVEL_11_0;
    const Vec4 WinRenderer::clearColor = Vec4(0.5, 0.5, 0.5, 1.0);


    WinRenderer::WinRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals) :
        BaseRenderer(eventSystem, globals),
		dxgiFactory(nullptr),
		dxgiAdapter(nullptr),
		device(nullptr),
		rtvHeap(nullptr),
		srvHeap(nullptr),
        frameContext{},
        frameIndex(0),
        currentFramebufferWidth(0),
        currentFramebufferHeight(0),
        commandQueue(nullptr),
        commandList(nullptr),
        frameFence(nullptr),
        frameFenceEvent(NULL),
        frameFenceLastValue(0)
    {
#ifdef DX12_ENABLE_DEBUG_LAYER
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
        }
#endif
    }

    WinRenderer::~WinRenderer()
    {
#ifdef DX12_ENABLE_DEBUG_LAYER
        ComPtr<IDXGIDebug1> pDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL));
        }
#endif
    }

    bool WinRenderer::init(const WinRendererParameters rendererParameters)
    {
        if (!BRWL_VERIFY(BaseRenderer::init(rendererParameters), BRWL_CHAR_LITERAL("Failed to init BaseRenderer")))
        {
            return false;
        }

        // Initialize Direct3D
        if (!createDevice(params->initialDimensions.width, params->initialDimensions.height))
        {
            destroyDevice();
            return false;;
        }

#ifdef BRWL_USE_DEAR_IM_GUI

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplWin32_Init(params->hWnd);
        ImGui_ImplDX12_Init(device, NUM_FRAMES_IN_FLIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, srvHeap, srvHeap->GetCPUDescriptorHandleForHeapStart(), srvHeap->GetGPUDescriptorHandleForHeapStart());

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.txt' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);
#endif // BRWL_USE_DEAR_IM_GUI
        return true;
    }

    void WinRenderer::render()
    {
        if (!currentFramebufferHeight || !currentFramebufferWidth)
        {
            logger->info(BRWL_CHAR_LITERAL("Nothing to render, framebuffer too small."));
            return;
        }

        BaseRenderer::render();
        // Create render data
#ifdef BRWL_USE_DEAR_IM_GUI

        // Render Gui last
        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        MakeTestUI();
        ImGui::Render();
#endif
        // Render

        FrameContext* frameCtxt = waitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtxt->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->Reset(frameCtxt->CommandAllocator.Get(), nullptr);
        commandList->ResourceBarrier(1, &barrier);
        commandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], (float*)&clearColor, 0, nullptr);
        commandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        commandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());

        // ========= START DRAW SCENE ========= //

#ifdef BRWL_USE_DEAR_IM_GUI
        BaseRenderer::render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#endif // BRWL_USE_DEAR_IM_GUI

        // =========  END DRAW SCENE  ========= //
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &barrier);
        commandList->Close();

        ID3D12CommandList* const ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(1, ppCommandLists);

        const HRESULT result = g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
        {
            // TODO: test this https://docs.microsoft.com/en-us/windows/uwp/gaming/handling-device-lost-scenarios
            const BRWL_CHAR* msg = nullptr;
            if (result == DXGI_ERROR_DEVICE_REMOVED) msg = BRWL_CHAR_LITERAL("DEVICE REMOVED!");
            if (result == DXGI_ERROR_DEVICE_RESET) msg = BRWL_CHAR_LITERAL("DEVICE RESET!");
            {
                Logger::ScopedMultiLog m(logger.get(), Logger::LogLevel::ERROR);
                logger->error(msg, &m);
                logger->error(GetDeviceRemovedReasonString(device->GetDeviceRemovedReason()), &m);
            }

        }

        UINT64 fenceValue = frameFenceLastValue + 1;
        commandQueue->Signal(frameFence.Get(), fenceValue);
        frameFenceLastValue = fenceValue;
        frameCtxt->FenceValue = fenceValue;


        if (!dxgiFactory->IsCurrent())
        {
            // TODO: FLUSH?
            logger->warning(BRWL_CHAR_LITERAL("Graphics adapters changed! Rescanning..."));
            destroyDevice();
            createDevice(currentFramebufferWidth, currentFramebufferHeight);
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
        }
    }

    void WinRenderer::destroy(bool force /*= false*/)
    {

        waitForLastSubmittedFrame();
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        if (GImGui != NULL)
        {
            ImGui::DestroyContext();
        }

        destroyDevice();

        BaseRenderer::destroy(force);
    }

#pragma region IMGUI COPY

    // Helper functions

    bool WinRenderer::createDevice(unsigned int framebufferWidth /*=0 */, unsigned int framebufferHeight /*=0 */)
    {
#ifndef __dxgi1_6_h__
#error Dude please get a Windows 10 and update, this was like 2018
#endif

        UINT flags = 0;
#if defined(_DEBUG)
        flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
        ComPtr<IDXGIFactory2> dxgiFactory2;

        if (!BRWL_VERIFY(CreateDXGIFactory2(flags, IID_PPV_ARGS(dxgiFactory2.ReleaseAndGetAddressOf())) == S_OK, BRWL_CHAR_LITERAL("Failed to create DXGIFactory2.")))
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
            for (unsigned int i = 0; (enumerationResult = dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(dxgiAdapter4.ReleaseAndGetAddressOf()))) == S_OK; ++i)
            {
                DXGI_ADAPTER_DESC1 desc;
                if (!BRWL_VERIFY(SUCCEEDED(dxgiAdapter4->GetDesc1(&desc)), BRWL_CHAR_LITERAL("Failed to get adapter description.")))
                {
                    return false;
                }

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

                // Create device and keep it if it supports the requested feature level
                if (BRWL_VERIFY(SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), featureLevel, IID_PPV_ARGS(device.ReleaseAndGetAddressOf()))), BRWL_CHAR_LITERAL("Failed to create D3D12Device.")))
                {
                    Logger::LogLevel logLvl = Logger::LogLevel::INFO;
                    // check root signature version 1_1 support
                    D3D12_FEATURE_DATA_ROOT_SIGNATURE highestVersion = { D3D_ROOT_SIGNATURE_VERSION_1_1 };
                    const BRWL_CHAR* errorMsg = nullptr;
                    if (!SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &highestVersion, sizeof(highestVersion))), errorMsg)
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

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = numBackBuffers;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (!BRWL_VERIFY(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap))), BRWL_CHAR_LITERAL("Failed to create render target view descriptor heap.")))
            {
                return false;
            }

            SIZE_T rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < numBackBuffers; i++)
            {
                g_mainRenderTargetDescriptor[i] = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};

            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (!BRWL_VERIFY(SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap))), BRWL_CHAR_LITERAL("Failed to create srv descriptor heap.")))
            {
                return false;
            }
        }

        {
            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;
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
        if( dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), params->hWnd, &sd, NULL, NULL, &swapChain1) != S_OK ||
            swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        g_pSwapChain->SetMaximumFrameLatency(numBackBuffers);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        
        createRenderTargets();
        currentFramebufferWidth = framebufferWidth;
        currentFramebufferHeight = framebufferHeight;
        return true;
    }

    void WinRenderer::destroyDevice()
    {
        destroyRenderTargets();
        for (UINT i = 0; i < numBackBuffers; i++)
        {
            g_mainRenderTargetDescriptor[i] = { 0 };
        }

        g_pSwapChain = nullptr;
        if (g_hSwapChainWaitableObject != NULL)
        {
            CloseHandle(g_hSwapChainWaitableObject);
            g_hSwapChainWaitableObject = NULL;
        }

        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        {
            frameContext[i].CommandAllocator = nullptr;
            frameContext[i].FenceValue = 0;
        }

        commandQueue = nullptr;
        commandList = nullptr;
        rtvHeap = nullptr;
        srvHeap = nullptr;
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
            g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            device->CreateRenderTargetView(pBackBuffer.Get(), NULL, g_mainRenderTargetDescriptor[i]);
            g_mainRenderTargetResource[i] = pBackBuffer;
            g_mainRenderTargetResource[i]->SetName(BRWL_CHAR_LITERAL("RenderTarget"));
        }
    }

    void WinRenderer::destroyRenderTargets()
    {
        waitForLastSubmittedFrame();

        for (UINT i = 0; i < numBackBuffers; i++)
        {
            g_mainRenderTargetResource[i] = nullptr;
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
        UINT nextFrameIndex = frameIndex + 1;
        frameIndex = nextFrameIndex;

        HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
        DWORD numWaitableObjects = 1;

        FrameContext* frameCtxt = &frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
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
            createDevice(currentFramebufferWidth, currentFramebufferHeight);
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            return;
        }

        destroyRenderTargets();

        DXGI_SWAP_CHAIN_DESC1 sd;
        HRESULT success = g_pSwapChain->GetDesc1(&sd);
        BRWL_EXCEPTION(SUCCEEDED(success), BRWL_CHAR_LITERAL("Failed to retrieve swap chain description."));
        success = g_pSwapChain->ResizeBuffers(sd.BufferCount, width, height, sd.Format, sd.Flags);
        BRWL_EXCEPTION(SUCCEEDED(success), BRWL_CHAR_LITERAL("Failed to resize swap chain."));
        createRenderTargets();

        //sd.Width = width;
        //sd.Height = height;
        //g_pSwapChain->Release();

        //CloseHandle(g_hSwapChainWaitableObject);

        //ComPtr<IDXGISwapChain1> swapChain1;
        //dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &sd, NULL, NULL, &swapChain1);
        //swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
        //swapChain1->Release();
        //dxgiFactory->Release();

        //g_pSwapChain->SetMaximumFrameLatency(numBackBuffers);

        //g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        assert(g_hSwapChainWaitableObject != NULL);
    }

    void WinRenderer::OnFramebufferResize(int width, int height)
    {
        width = Utils::max(width, 1);
        height = Utils::max(height, 1);
        currentFramebufferWidth = width;
        currentFramebufferHeight = height;
        waitForLastSubmittedFrame();
        ImGui_ImplDX12_InvalidateDeviceObjects();
        resizeSwapChain(params->hWnd, width, height);
        ImGui_ImplDX12_CreateDeviceObjects();
        render();
    }

#pragma endregion //IMGUI COPY

} // namespace PAL


BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS

