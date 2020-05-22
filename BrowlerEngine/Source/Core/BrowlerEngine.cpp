#include "BrowlerEngine.h"

#include "Common/Logger.h"
#include "Timer.h"
#include "Window.h"
#include "Renderer/Renderer.h"


BRWL_NS


Engine::Engine( TickProvider* tickProvider, PlatformGlobals* globals) :
	logger(nullptr),
	eventSystem(nullptr),
	window(nullptr),
	renderer(nullptr),
	isInitialized(false),
	tickProvider(tickProvider),
	globals(globals),
	runMode(MetaEngine::EngineRunMode::META_ENGINE_MAIN_THREAD)
{ }

Engine::~Engine()
{ }

bool Engine::init(const char* settingsFile)
{
	// make sure we are clean
	close();

	if (!internalInit(settingsFile))
	{
		close();
		return false;
	}

	time->start();

	return true;
}

void Engine::threadInit()
{
	window->create(0, 0, 600, 1200);
	isInitialized = true;
}

void Engine::update()
{
	BRWL_CHAR msg[20];
	BRWL_SNPRINTF(msg, countof(msg), BRWL_CHAR_LITERAL("%.f"), time->getTimeF());
	Engine::LogInfo(msg);
	if (window && runMode == MetaEngine::EngineRunMode::DETATCHED)
	{	// only process messages if they are not received from the parent window
		window->processPlatformMessages();
	}
}

void Engine::shutdown()
{
	// signal that we want to close the program after the end of the frame
	//glfwSetWindowShouldClose(app->window->window, true);
}

bool Engine::shouldClose()
{
	return false;//return  isInitialized ? glfwWindowShouldClose(window->window) : false;
}

void Engine::close()
{	
	isInitialized = false;
	if (time) time->stop();

	//// Input
	//if (input)
	//{
	//	input->deactivate();
	//	input = nullptr;
	//}

	//// Hierarchy
	//hierarchy = nullptr;

	//// Mesh Registry
	//if (meshRegistry) {
	//	meshRegistry = nullptr;
	//}

	//// Mesh Registry
	//if (textureRegistry) {
	//	textureRegistry = nullptr;
	//}
	// Renderer

	if (renderer)
	{
		renderer->destroy();
		renderer = nullptr;
	}

	// Window
	if (window)
	{
		window->destroy();
		window = nullptr;
	}

	//// Event System
	if (eventSystem) {
		if (eventSystem->hasAnyListeners())
		{
			LogWarning(BRWL_CHAR_LITERAL("The event bus still had listeners registered on application shutdown!"));
		}

		eventSystem = nullptr;
	}

	// game timer
	time = nullptr;

	// Logger
	//Not destroing the logger here. It should be destroyed when the application object is destroyed  or when a new logger is created

	//// Settings
	//settings = nullptr;
}

bool Engine::internalInit(const char* settingsFile)
{
	//// Load Settings
	//INIReader reader(settingsFile);
	//settings = std::make_unique<Settings>();
	//settings->read(reader);

	// Logger
	logger = std::make_unique<Logger>();

	//if (!VERIFY(settings != nullptr, "Invalid argument"))
	//{
	//	return false;
	//}

	// game timer
	time = std::make_unique<Timer>(tickProvider);

	// Event System
	eventSystem = std::make_unique<CoreEventSystem>();
	
	//// Window
	window = std::make_unique<Window>(globals, eventSystem.get());

	// Renderer
	renderer = std::make_unique<RENDERER::Renderer>(eventSystem.get());
	renderer->setLogger(logger);
	if (!BRWL_VERIFY(renderer->init(), BRWL_CHAR_LITERAL("Failed to initialize renderer!")))
	{
		return false;
	}

	//if (!VERIFY(window->create(
	//	settings->window.width,
	//	settings->window.height,
	//	settings->window.title.value.c_str(),
	//	settings->window.allowResize,
	//	settings->window.fullscreen), "Failed to create window"))
	//{
	//	return false;
	//}

	//// Texture Registry
	//textureRegistry = std::make_unique<TextureRegistry>();

	//// Mesh Registry
	//meshRegistry = std::make_unique<MeshRegistry>();
	//meshRegistry->setLogger(logger);

	//// Hierarchy
	//hierarchy = std::make_unique<Hierarchy>();



	//// Input
	//input = std::make_unique<InputManager>(true);

	return true;
}

void Engine::LogDebug(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->debug(msg); }
void Engine::LogInfo(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->info(msg); }
void Engine::LogWarning(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->warning(msg); }
void Engine::LogError(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->error(msg); }

BRWL_NS_END
