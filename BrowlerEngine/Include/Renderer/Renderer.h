#pragma once

#include "RendererFwd.h"
#include "PAL/WinRenderer.h"
#include "Common/EventSystem.h"
#include "Events.h"
#include "Common/Logger.h"

BRWL_RENDERER_NS


class Renderer : public PAL::PlatformRenderer
{
public:
	Renderer(EventBusSwitch<Event>* eventSystem);
	virtual ~Renderer();

	void setLogger(std::shared_ptr<Logger> logger) { this->logger = logger; }
	bool init();
	void destroy();

protected:
	EventBusSwitch<Event>* eventSystem;
	size_t windowResizeEventHandle;
	std::shared_ptr<Logger> logger;
};


BRWL_RENDERER_NS_END