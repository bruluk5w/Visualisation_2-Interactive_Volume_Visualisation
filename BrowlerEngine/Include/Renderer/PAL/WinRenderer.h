#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

#include "BaseRenderer.h"
#include "Common/BrwlMathFwd.h"

BRWL_RENDERER_NS

class Visualization2Renderer;

namespace PAL
{

	class WinRenderer : public BaseRenderer
	{
		friend class Visualization2Renderer;
	public:
		static const D3D_FEATURE_LEVEL	featureLevel;
		static const unsigned int numBackBuffers = 3;

		WinRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals);
		virtual ~WinRenderer();
		virtual bool init(const WinRendererParameters params) override;
		virtual void render() override;
		virtual void destroy(bool force = true) override;
	protected:
		static const Vec4 clearColor;

		ComPtr<IDXGIFactory6>	dxgiFactory;
		ComPtr<IDXGIAdapter4>	dxgiAdapter;
		ComPtr<ID3D12Device>	device;
		ComPtr<ID3D12DescriptorHeap>	rtvHeap;
		ComPtr<ID3D12DescriptorHeap>	srvHeap;

		bool createDevice(unsigned int framebufferWidth = 0, unsigned int framebufferHeight = 0);
		void destroyDevice();

		struct FrameContext
		{
			ComPtr<ID3D12CommandAllocator>	CommandAllocator;
			uint64_t						FenceValue;
		};

		static const int			NUM_FRAMES_IN_FLIGHT = 3;
		FrameContext				frameContext[NUM_FRAMES_IN_FLIGHT];
		unsigned int				frameIndex;
		unsigned int				currentFramebufferWidth;
		unsigned int				currentFramebufferHeight;


		void createRenderTargets();
		void destroyRenderTargets();
		void waitForLastSubmittedFrame();
		FrameContext* waitForNextFrameResources();
		void resizeSwapChain(HWND hWnd, int width, int height);


		ComPtr<ID3D12CommandQueue>			commandQueue;
		ComPtr<ID3D12GraphicsCommandList>	commandList;
		ComPtr<ID3D12Fence>					frameFence;
		HANDLE								frameFenceEvent;
		uint64_t							frameFenceLastValue;
		ComPtr<IDXGISwapChain3>				g_pSwapChain = nullptr;
		HANDLE								g_hSwapChainWaitableObject = NULL;
		ComPtr<ID3D12Resource>				g_mainRenderTargetResource[numBackBuffers] = { nullptr };
		D3D12_CPU_DESCRIPTOR_HANDLE			g_mainRenderTargetDescriptor[numBackBuffers] = { 0 };

		// Inherited via BaseRenderer
		virtual void OnFramebufferResize(int width, int height) override;
	};
}


BRWL_RENDERER_NS_END

#endif
