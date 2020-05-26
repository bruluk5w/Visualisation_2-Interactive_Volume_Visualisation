#include "PAL/WinRenderer.h"

#ifdef BRWL_PLATFORM_WINDOWS

// DX12 additional headers
#include <DirectXMath.h>
#include <PAL/d3dx12.h>

#include "PAL/WinRendererParameters.h"

#ifdef BRWL_USE_DEAR_IM_GUI
#include "UI/ImGui/imgui.h"
#include "PAL/imgui_impl_dx12.h"
#include "PAL/imgui_impl_win32.h"
#include "UI/TestUi.h"
#endif

BRWL_RENDERER_NS

namespace PAL
{

    WinRenderer::WinRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals) :
        BaseRenderer(eventSystem, globals)
    {
#ifdef DX12_ENABLE_DEBUG_LAYER
        ComPtr<ID3D12Debug> debugController0;
        if (BRWL_VERIFY(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController0)) >= 0, BRWL_CHAR_LITERAL("Failed to enable D3D debug layer.")))
        {
            debugController0->EnableDebugLayer();
#ifdef _DEBUG
            ComPtr<ID3D12Debug1> debugController1;
            if (BRWL_VERIFY(debugController0->QueryInterface(IID_PPV_ARGS(&debugController1)), BRWL_CHAR_LITERAL("Failed to enable GPU-based validation.")))
            {
                debugController1->SetEnableGPUBasedValidation(true);
            }
#endif
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
        if (!CreateDevice(params->hWnd))
        {
            DestroyDevice();
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
        ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
            DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
            g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

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
        BaseRenderer::render();

#ifdef BRWL_USE_DEAR_IM_GUI

        // Render Gui last
        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

       // MakeTestUI();

        // Rendering
        static const ImVec4 clearColor(0.45f, 0.55f, 0.60f, 1.00f);

        FrameContext* frameCtxt = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtxt->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        g_pd3dCommandList->Reset(frameCtxt->CommandAllocator.Get(), nullptr);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], (float*)&clearColor, 0, nullptr);
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList.Get());
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        ID3D12CommandList* const ppCommandLists[] = { g_pd3dCommandList.Get() };
        g_pd3dCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence.Get(), fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtxt->FenceValue = fenceValue;

#endif // BRWL_USE_DEAR_IM_GUI

    }

    void WinRenderer::destroy(bool force /*= false*/)
    {

        WaitForLastSubmittedFrame();
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        DestroyDevice();

        BaseRenderer::destroy(force);
    }

#pragma region IMGUI COPY

    // Helper functions

    bool WinRenderer::CreateDevice(HWND hWnd)
    {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC1 sd;
        {
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = NUM_BACK_BUFFERS;
            sd.Width = 0;
            sd.Height = 0;
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

#ifdef DX12_ENABLE_DEBUG_LAYER
        ID3D12Debug* pdx12Debug = NULL;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        {
            pdx12Debug->EnableDebugLayer();
            pdx12Debug->Release();
        }
#endif

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
            return false;

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = NUM_BACK_BUFFERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
                return false;

            SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
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
            if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                return false;
        }

        {
            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;
            if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
                return false;
        }

        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
            HRESULT res = g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator));
            if (res != S_OK)
                return false;
        }

        if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator.Get(), NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
            g_pd3dCommandList->Close() != S_OK)
            return false;

        if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
            return false;

        g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (g_fenceEvent == NULL)
            return false;

        {
            ComPtr<IDXGIFactory4> dxgiFactory;
            ComPtr<IDXGISwapChain1> swapChain1;
            UINT flags = 0;
#if defined(_DEBUG)
            flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
            if (CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory)) != S_OK ||
                dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue.Get(), hWnd, &sd, NULL, NULL, &swapChain1) != S_OK ||
                swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
                return false;
            g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
            g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        }

        CreateRenderTarget();
        return true;
    }

    void WinRenderer::DestroyDevice()
    {
        DestroyRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
        if (g_hSwapChainWaitableObject != NULL) { CloseHandle(g_hSwapChainWaitableObject); }
        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
            if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
        if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
        if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
        if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
        if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
        if (g_fence) { g_fence->Release(); g_fence = nullptr; }
        if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
        IDXGIDebug1* pDebug = NULL;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
            pDebug->Release();
        }
#endif
    }

    void WinRenderer::CreateRenderTarget()
    {
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            ComPtr<ID3D12Resource> pBackBuffer = nullptr;
            g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), NULL, g_mainRenderTargetDescriptor[i]);
            g_mainRenderTargetResource[i] = pBackBuffer;
        }
    }

    void WinRenderer::DestroyRenderTarget()
    {
        WaitForLastSubmittedFrame();

        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
    }

    void WinRenderer::WaitForLastSubmittedFrame()
    {
        FrameContext* frameCtxt = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

        UINT64 fenceValue = frameCtxt->FenceValue;
        if (fenceValue == 0)
            return; // No fence was signaled

        frameCtxt->FenceValue = 0;
        if (g_fence->GetCompletedValue() >= fenceValue)
            return;

        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }

    WinRenderer::FrameContext* WinRenderer::WaitForNextFrameResources()
    {
        UINT nextFrameIndex = g_frameIndex + 1;
        g_frameIndex = nextFrameIndex;

        HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
        DWORD numWaitableObjects = 1;

        FrameContext* frameCtxt = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
        UINT64 fenceValue = frameCtxt->FenceValue;
        if (fenceValue != 0) // means no fence was signaled
        {
            frameCtxt->FenceValue = 0;
            g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
            waitableObjects[1] = g_fenceEvent;
            numWaitableObjects = 2;
        }

        WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

        return frameCtxt;
    }

    void WinRenderer::ResizeSwapChain(HWND hWnd, int width, int height)
    {
        DXGI_SWAP_CHAIN_DESC1 sd;
        g_pSwapChain->GetDesc1(&sd);
        sd.Width = width;
        sd.Height = height;

        IDXGIFactory4* dxgiFactory = NULL;
        g_pSwapChain->GetParent(IID_PPV_ARGS(&dxgiFactory));

        g_pSwapChain->Release();
        CloseHandle(g_hSwapChainWaitableObject);

        IDXGISwapChain1* swapChain1 = NULL;
        dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue.Get(), hWnd, &sd, NULL, NULL, &swapChain1);
        swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
        swapChain1->Release();
        dxgiFactory->Release();

        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        assert(g_hSwapChainWaitableObject != NULL);
    }

    void WinRenderer::OnFramebufferResize(int width, int height)
    {
        WaitForLastSubmittedFrame();
        ImGui_ImplDX12_InvalidateDeviceObjects();
        DestroyRenderTarget();
        ResizeSwapChain(params->hWnd, width, height);
        CreateRenderTarget();
        ImGui_ImplDX12_CreateDeviceObjects();
    }

#pragma endregion //IMGUI COPY

} // namespace PAL


BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS

