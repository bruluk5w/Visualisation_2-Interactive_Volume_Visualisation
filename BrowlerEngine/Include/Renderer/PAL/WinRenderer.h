#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

#include "BaseRenderer.h"
#include "RtvDescriptorHeap.h"
#include "PAL/DescriptorHeap.h"

BRWL_RENDERER_NS


class Visualization2Renderer;

namespace PAL
{
	void HandleDeviceRemoved(HRESULT result, ID3D12Device* device, ::BRWL::Logger& logger);

	extern  DXGI_FORMAT g_RenderTargetFormat;

	class WinRenderer : public BaseRenderer
	{
		// TODO: ugly but no time to write that properly now
		friend class Visualization2Renderer;
		friend class MainShader;
	public:
		static const D3D_FEATURE_LEVEL	featureLevel;
		static const unsigned int numBackBuffers = 3;

		WinRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals);
		virtual ~WinRenderer();
		virtual void draw() override;
		virtual void destroy(bool force = true) override;
		virtual void setVSync(bool enable) override { vSync = enable; }
		virtual bool getVSync() const override { return vSync; }
		void waitForLastSubmittedFrame();
		DescriptorHeap& getSrvHeap() { return srvHeap; }
	protected:
		virtual bool internalInit(const WinRendererParameters params) override;
		virtual void nextFrame() override;
		virtual void appRender() override;

		static const Vec4 clearColor;

		ComPtr<IDXGIFactory6>	dxgiFactory;
		ComPtr<IDXGIAdapter4>	dxgiAdapter;
		ComPtr<ID3D12Device>	device;
		RtvDescriptorHeap		rtvHeap;
		DescriptorHeap			srvHeap;

		bool createDevice(HWND hWnd, unsigned int framebufferWidth = 0, unsigned int framebufferHeight = 0);
		void destroyDevice();

		struct FrameContext
		{
			ComPtr<ID3D12CommandAllocator>	CommandAllocator;
			uint64_t						FenceValue;
		};

		static const int	NUM_FRAMES_IN_FLIGHT = 3;
		bool				vSync;
		FrameContext		frameContext[NUM_FRAMES_IN_FLIGHT];
		size_t				frameIndex;
		

		void createRenderTargets();
		void destroyRenderTargets();
		FrameContext* waitForNextFrameResources();
		FrameContext* getCurrentFrameContext();
		FrameContext* getNextFrameContext();
		void resizeSwapChain(HWND hWnd, int width, int height);


		ComPtr<ID3D12CommandQueue>			commandQueue;
		ComPtr<ID3D12GraphicsCommandList>	commandList;
		ComPtr<ID3D12Fence>					frameFence;
		HANDLE								frameFenceEvent;
		uint64_t							frameFenceLastValue;
		ComPtr<IDXGISwapChain3>				swapChain = nullptr;
		HANDLE								swapChainWaitableObject = NULL;
		ComPtr<ID3D12Resource>				mainRenderTargetResource[numBackBuffers] = { nullptr };
		RtvDescriptorHeap::Handle			mainRenderTargetDescriptor[numBackBuffers] = { };

		// Inherited via BaseRenderer
		virtual void OnFramebufferResize() override;
		virtual std::unique_ptr<BaseTextureManager> makeTextureManager() override;

	};
}


BRWL_RENDERER_NS_END

#endif
