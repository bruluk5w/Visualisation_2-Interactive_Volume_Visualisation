#pragma once

#include "Common/EventSystem.h"
#include "Events.h"
#include "Common/GlobalsFwd.h"
#include "RendererFwd.h"
#include "RendererParametersFwd.h"

BRWL_NS


class Logger;


BRWL_NS_END

BRWL_RENDERER_NS


class BaseRenderer
{
public:
	BaseRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals);
	virtual ~BaseRenderer();

	void setLogger(std::shared_ptr<Logger> logger) { this->logger = logger; }
	virtual bool init(const RendererParameters params);
	virtual void render();
	// may be called multiple times
	virtual void destroy(bool force = false);
	bool isInitialized() const { return initialized; }

protected:
	virtual void OnFramebufferResize(int width, int height) = 0;

	bool initialized;
	EventBusSwitch<Event>* eventSystem;
	size_t windowResizeEventHandle;
	std::shared_ptr<Logger> logger;
	PlatformGlobals* globals;
	std::unique_ptr<RendererParameters> params;
};


BRWL_RENDERER_NS_END