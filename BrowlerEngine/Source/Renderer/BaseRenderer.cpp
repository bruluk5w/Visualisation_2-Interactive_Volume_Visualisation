#include "BaseRenderer.h"
#include "RendererParameters.h"
#include "Common/Logger.h"
#include "AppRenderer.h"

#include "PAL/WinRenderer.h"

BRWL_RENDERER_NS


BaseRenderer::BaseRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals) :
	initialized(false),
	eventSystem(eventSystem),
	windowResizeEventHandle(0),
	logger(nullptr),
	globals(globals),
	params(nullptr),
	appRenderer(nullptr)
{ }

BaseRenderer::~BaseRenderer()
{
	if (!BRWL_VERIFY(!initialized, BRWL_CHAR_LITERAL("Renderer is deleted but is still operating on a window!")))
	{
		destroy();
	}
}

bool BaseRenderer::init(const RendererParameters params)
{
	if (initialized)
	{
		if (!BRWL_VERIFY(memcmp(&params, this->params.get(), sizeof(RendererParameters)) == 0, BRWL_CHAR_LITERAL("Renderer is being initialized on a new window while it is still attached to another window!")))
		{
			destroy();
		}
	}

	if (!initialized)
	{
		this->params = std::make_unique<RendererParameters>(params);
		windowResizeEventHandle = eventSystem->registerListener(Event::WINDOW_RESIZE, [this](Event, void* param) -> bool
			{
				logger->info(BRWL_CHAR_LITERAL("Resizing Framebuffer"));
				OnFramebufferResize(castParam<Event::WINDOW_RESIZE>(param)->width, castParam<Event::WINDOW_RESIZE>(param)->height);
				return false;
			});

		if (appRenderer && !BRWL_VERIFY(appRenderer->rendererInit(), BRWL_CHAR_LITERAL("Failed to initialize the app renderer")))
		{
			destroy(true);
		}
		else
		{
			initialized = true;
		}
	}
	
	return initialized;
}

void BaseRenderer::preRender()
{
	if (appRenderer)
	{
		appRenderer->rendererInit();
		appRenderer->preRender();
	}

}

void BaseRenderer::render()
{
	if (appRenderer)
	{
		BRWL_EXCEPTION(appRenderer->isInitalized(), BRWL_CHAR_LITERAL("Invalid renderer state."));
		appRenderer->render();
		appRenderer->draw(static_cast<Renderer*>(this));
	}
}

void BaseRenderer::draw()
{
	if (appRenderer)
	{
		BRWL_EXCEPTION(appRenderer->isInitalized(), BRWL_CHAR_LITERAL("Invalid renderer state."));
		appRenderer->draw(static_cast<Renderer*>(this));
	}
}

void BaseRenderer::destroy(bool force /*= false*/)
{
	if (initialized || force)
	{
		bool success = eventSystem->unregisterListener(Event::WINDOW_RESIZE, windowResizeEventHandle);
		BRWL_CHECK(success, BRWL_CHAR_LITERAL("Failed to unregister a listener!"));
		if (appRenderer)
		{
			appRenderer->rendererDestroy();
		}
		initialized = false;
	}
}


BRWL_RENDERER_NS_END