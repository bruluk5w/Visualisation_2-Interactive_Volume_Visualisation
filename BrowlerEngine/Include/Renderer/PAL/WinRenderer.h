#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

#include "BaseRenderer.h"
#include "DescriptorHeap.h"

BRWL_RENDERER_NS


class Visualization2Renderer;

namespace PAL
{
	void HandleDeviceRemoved(HRESULT result, ID3D12Device* device, ::BRWL::Logger& logger);

	extern  DXGI_FORMAT g_RenderTargetFormat;

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
		virtual void draw() override;
		virtual void destroy(bool force = true) override;
	protected:

		static const Vec4 clearColor;

		ComPtr<IDXGIFactory6>	dxgiFactory;
		ComPtr<IDXGIAdapter4>	dxgiAdapter;
		ComPtr<ID3D12Device>	device;
		DescriptorHeap	rtvHeap;
		DescriptorHeap	srvHeap;

		bool createDevice(HWND hWnd, unsigned int framebufferWidth = 0, unsigned int framebufferHeight = 0);
		void destroyDevice();

		struct FrameContext
		{
			ComPtr<ID3D12CommandAllocator>	CommandAllocator;
			uint64_t						FenceValue;
		};

		static const int			NUM_FRAMES_IN_FLIGHT = 3;
		FrameContext				frameContext[NUM_FRAMES_IN_FLIGHT];
		size_t						frameIndex;
		

		void createRenderTargets();
		void destroyRenderTargets();
		void waitForLastSubmittedFrame();
		FrameContext* waitForNextFrameResources();
		FrameContext* getCurrentFrameContext();
		void resizeSwapChain(HWND hWnd, int width, int height);


		ComPtr<ID3D12CommandQueue>			commandQueue;
		ComPtr<ID3D12GraphicsCommandList>	commandList;
		ComPtr<ID3D12Fence>					frameFence;
		HANDLE								frameFenceEvent;
		uint64_t							frameFenceLastValue;
		ComPtr<IDXGISwapChain3>				swapChain = nullptr;
		HANDLE								swapChainWaitableObject = NULL;
		ComPtr<ID3D12Resource>				mainRenderTargetResource[numBackBuffers] = { nullptr };
		DescriptorHeap::Handle				mainRenderTargetDescriptor[numBackBuffers] = { };

		// Inherited via BaseRenderer
		virtual void OnFramebufferResize() override;

		DescriptorHeap::Handle fontTextureDescriptorHandle;

#ifdef _DEBUG
		size_t frameIdxChangeListenerHandle = 0;
		bool OnFrameIdxChange(size_t newFrameIdx) { srvHeap.setFrameIdx(newFrameIdx); srvHeap.setFrameIdx(newFrameIdx); return false; }
#endif
	};
}


BRWL_RENDERER_NS_END

#endif
