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
class BaseTextureManager;

class BaseRenderer
{
public:
	BaseRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals);
	virtual ~BaseRenderer();

	void setLogger(std::shared_ptr<Logger> logger) { this->logger = logger; }
	bool init(const RendererParameters params);
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
	EventBusSwitch<Event>* getEventSystem() { return eventSystem; }
	AppRenderer* getAppRenderer() { return appRenderer.get(); }
	BaseTextureManager* getTextureManager() { return textureManager.get(); }
	

	void setCamera(Camera* newCamera);
	Camera* getCamera() const { return camera; }
	void getFrameBufferSize(unsigned int& width, unsigned int& height) { width = currentFramebufferWidth; height = currentFramebufferHeight; }
	virtual void setVSync(bool enable) = 0;
	virtual bool getVSync() const = 0;

	void log(const BRWL_CHAR* msg, Logger::LogLevel level, Logger::ScopedMultiLog* multiLog = nullptr) const {
		if (logger) logger->log(msg, level, multiLog);
	}

	// todo: remove if not needed anymore
	bool anyTextureBecameResident;

protected:
	virtual bool internalInitStep0(const RendererParameters params); //!< returns true if succeeded else false;
	virtual bool internalInitStep1(const RendererParameters params); //!< returns true if succeeded else false;
	virtual void nextFrame() { };
	virtual void appRender();
	virtual void OnFramebufferResize() = 0;
	virtual std::unique_ptr<BaseTextureManager> makeTextureManager() = 0;

	bool							initialized;
	EventBusSwitch<Event>*			eventSystem;
	size_t							windowResizeEventHandle;
	std::shared_ptr<Logger>			logger;
	PlatformGlobals*				globals;
	std::unique_ptr<RendererParameters> params;
	std::unique_ptr<BaseTextureManager>	textureManager;
	std::shared_ptr<AppRenderer>	appRenderer;
	Camera*							camera;
	unsigned int					currentFramebufferWidth;
	unsigned int					currentFramebufferHeight;
};


BRWL_RENDERER_NS_END