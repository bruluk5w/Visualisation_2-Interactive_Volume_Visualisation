#include "BaseRenderer.h"
#include "RendererParameters.h"
#include "TextureManager.h"
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
	textureManager(nullptr),
	appRenderer(nullptr),
	currentFramebufferWidth(0),
	currentFramebufferHeight(0),
	anyTextureBecameResident(false)
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
		if (!BRWL_VERIFY(memcmp(&params, this->params.get(), sizeof(RendererParameters)) == 0, BRWL_CHAR_LITERAL("Renderer is being initialized on a window while it is still attached to another window!")))
		{
			destroy();
		}
	}

	if (!initialized)
	{
		// tdodo: pass params without copy
		return internalInitStep0(params) && internalInitStep1(params);
	}

	return false;
}

bool BaseRenderer::internalInitStep0(const RendererParameters params)
{
	if (BRWL_VERIFY(!initialized, nullptr))
	{
		this->params = std::make_unique<RendererParameters>(params);

		this->textureManager = makeTextureManager();

		windowResizeEventHandle = eventSystem->registerListener(Event::WINDOW_RESIZE, [this](Event, void* param) -> bool
			{
				logger->info(BRWL_CHAR_LITERAL("Resizing Framebuffer"));
				currentFramebufferWidth = Utils::max<unsigned int>(currentFramebufferWidth, 1);
				currentFramebufferHeight = Utils::max<unsigned int>(currentFramebufferHeight, 1);

				OnFramebufferResize();
				return false;
			});
	}

	return true;
}

bool BaseRenderer::internalInitStep1(const RendererParameters params)
{
	if (BRWL_VERIFY(!initialized, nullptr))
	{
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
	if (!currentFramebufferHeight || !currentFramebufferWidth)
	{
		logger->debug(BRWL_CHAR_LITERAL("Nothing to render, framebuffer too small."));
		return;
	}

	nextFrame();
	// swap textures that are resident now
	anyTextureBecameResident = textureManager->promoteStagedTextures();
	appRender();
	textureManager->update();
}

void BaseRenderer::appRender()
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

		this->textureManager = nullptr;
		this->params = nullptr;

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