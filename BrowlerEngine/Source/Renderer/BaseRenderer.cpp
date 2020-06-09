#include "BaseRenderer.h"
#include "RendererParameters.h"
#include "Common/Logger.h"
#include "AppRenderer.h"

#include "PAL/WinRenderer.h"
#include "Camera.h"

BRWL_RENDERER_NS


BaseRenderer::BaseRenderer(EventBusSwitch<Event>* eventSystem, PlatformGlobals* globals) :
	initialized(false),
	eventSystem(eventSystem),
	windowResizeEventHandle(0),
	logger(nullptr),
	globals(globals),
	params(nullptr),
	appRenderer(nullptr),
	currentFramebufferWidth(0),
	currentFramebufferHeight(0)
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
				currentFramebufferWidth = castParam<Event::WINDOW_RESIZE>(param)->width;
				currentFramebufferHeight = castParam<Event::WINDOW_RESIZE>(param)->height;
				OnFramebufferResize();
				return false;
			});

		if (appRenderer && !appRenderer->isInitalized() && !BRWL_VERIFY(appRenderer->rendererInit(static_cast<Renderer*>(this)), BRWL_CHAR_LITERAL("Failed to initialize the app renderer")))
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
		appRenderer->rendererInit(static_cast<Renderer*>(this));
		appRenderer->preRender(static_cast<Renderer*>(this));
	}
}

void BaseRenderer::render()
{
	if (appRenderer)
	{
		BRWL_EXCEPTION(appRenderer->isInitalized(), BRWL_CHAR_LITERAL("Invalid renderer state."));
		appRenderer->render(static_cast<Renderer*>(this));
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
			appRenderer->rendererDestroy(static_cast<Renderer*>(this));
		}
		initialized = false;
	}
}

void BaseRenderer::setCamera(Camera* newCamera)
{
	camera = newCamera;
	if (camera != nullptr) {
		Logger::ScopedMultiLog ml(logger.get(), Logger::LogLevel::INFO);
		logger->info(BRWL_CHAR_LITERAL("Active camera changed to: "), &ml);
		logger->info(camera->getName().c_str(), &ml);
	}
	else
	{
		logger->warning(BRWL_CHAR_LITERAL("Camera has been removed!"));
	}
}


BRWL_RENDERER_NS_END