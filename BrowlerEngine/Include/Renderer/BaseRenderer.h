#pragma once // (c) 2020 Lukas Brunner

#include "Common/EventSystem.h"
#include "Events.h"
#include "Common/GlobalsFwd.h"
#include "RendererFwd.h"
#include "RendererParametersFwd.h"

BRWL_NS


class Logger;

BRWL_NS_END

BRWL_RENDERER_NS


class AppRenderer;
class Camera;

class BaseRenderer
{
public:
	BaseRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals);
	virtual ~BaseRenderer();

	void setLogger(std::shared_ptr<Logger> logger) { this->logger = logger; }
	virtual bool init(const RendererParameters params);
	virtual void preRender();
	void render();
	virtual void draw();
	// may be called multiple times
	virtual void destroy(bool force = false);
	bool isInitialized() const { return initialized; }
	template<typename AppRendererT, class... Types>
	void createAppRenderer(Types&&... args)
	{
		static_assert(std::is_base_of_v<AppRenderer, AppRendererT>);
		if (appRenderer) appRenderer = nullptr;
		appRenderer = std::make_shared<AppRendererT>(std::forward<Types>(args)...);
	}
	void setCamera(Camera* newCamera);
	Camera* getCamera() const { return camera; }
	void getFrameBufferSize(unsigned int& width, unsigned int& height) { width = currentFramebufferWidth; height = currentFramebufferHeight; }
	virtual void setVSync(bool enable) = 0;
	virtual bool getVSync() const = 0;
protected:
	virtual void platformRender();
	virtual void OnFramebufferResize() = 0;

	bool							initialized;
	EventBusSwitch<Event>*			eventSystem;
	size_t							windowResizeEventHandle;
	std::shared_ptr<Logger>			logger;
	PlatformGlobals*				globals;
	std::unique_ptr<RendererParameters> params;
	std::shared_ptr<AppRenderer>	appRenderer;
	Camera*							camera;
	unsigned int					currentFramebufferWidth;
	unsigned int					currentFramebufferHeight;


};


BRWL_RENDERER_NS_END