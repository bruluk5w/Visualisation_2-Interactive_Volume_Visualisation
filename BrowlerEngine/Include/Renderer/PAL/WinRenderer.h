#pragma once

#ifdef BRWL_PLATFORM_WINDOWS
#pragma once

#include "BaseRenderer.h"

BRWL_RENDERER_NS

namespace PAL
{

	class WinRenderer : public BaseRenderer
	{
	public:
		WinRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals);
		virtual bool init(const WinRendererParameters params) override;
		virtual void render() override;
		virtual void destroy(bool force = true) override;
	protected:

		// Helper methods
		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
		void WaitForLastSubmittedFrame();
		struct FrameContext;
		FrameContext* WaitForNextFrameResources();
		void ResizeSwapChain(HWND hWnd, int width, int height);

		struct FrameContext
		{
			ComPtr<ID3D12CommandAllocator> CommandAllocator;
			UINT64                  FenceValue;
		};

		// Data
		static const int			NUM_FRAMES_IN_FLIGHT = 3;
		FrameContext				g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
		UINT						g_frameIndex = 0;

		static const int					NUM_BACK_BUFFERS = 3;
		ComPtr<ID3D12Device>				g_pd3dDevice = nullptr;
		ComPtr<ID3D12DescriptorHeap>		g_pd3dRtvDescHeap = nullptr;
		ComPtr<ID3D12DescriptorHeap>		g_pd3dSrvDescHeap = nullptr;
		ComPtr<ID3D12CommandQueue>			g_pd3dCommandQueue = nullptr;
		ComPtr<ID3D12GraphicsCommandList>	g_pd3dCommandList = nullptr;
		ComPtr<ID3D12Fence>					g_fence = nullptr;
		HANDLE								g_fenceEvent = NULL;
		UINT64								g_fenceLastSignaledValue = 0;
		ComPtr<IDXGISwapChain3>				g_pSwapChain = nullptr;
		HANDLE								g_hSwapChainWaitableObject = NULL;
		ComPtr<ID3D12Resource>				g_mainRenderTargetResource[NUM_BACK_BUFFERS] = { nullptr };
		D3D12_CPU_DESCRIPTOR_HANDLE			g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = { 0 };

		// Inherited via BaseRenderer
		virtual void OnFramebufferResize(int width, int height) override;
	};
}


BRWL_RENDERER_NS_END

#endif
