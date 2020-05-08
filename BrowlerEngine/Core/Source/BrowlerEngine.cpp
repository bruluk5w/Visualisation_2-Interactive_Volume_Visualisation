#include "BrowlerEngine.h"

#include "Timer.h"

#include <iostream>

BRWL_NS


Engine::Engine( TickProvider* tickProvider, PlatformGlobals* globals) :
	isInitialized(false),
	tickProvider(tickProvider),
	globals(nullptr),
	runMode(MetaEngine::EngineRunMode::SYNCHRONIZED)
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

	return true;
}

void Engine::update()
{
	std::cout << time->getDeltaTimeF();
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
	//// Input
	//if (input)
	//{
	//	input->deactivate();
	//	input = nullptr;
	//}

	//// Hierarchy
	//hierarchy = nullptr;

	//// Renderer
	//if (renderer)
	//{
	//	renderer->destroy();
	//	renderer = nullptr;
	//}

	//// Mesh Registry
	//if (meshRegistry) {
	//	meshRegistry = nullptr;
	//}

	//// Mesh Registry
	//if (textureRegistry) {
	//	textureRegistry = nullptr;
	//}

	//// Window
	//if (window)
	//{
	//	window->destroy();
	//	window = nullptr;
	//}

	//// Event System
	//if (eventBus) {
	//	if (eventBus->hasAnyListeners())
	//	{
	//		logger->warning("The event bus still had listeners registered on application shutdown!");
	//	}

	//	eventBus = nullptr;
	//}

	// game timer
	time = nullptr;

	// Logger
	// Not destroing the logger here. It should be destroyed when the application object is destroyed  or when a new logger is created

	//// Settings
	//settings = nullptr;
}

bool Engine::internalInit(const char* settingsFile)
{
	//// Load Settings
	//INIReader reader(settingsFile);
	//settings = std::make_unique<Settings>();
	//settings->read(reader);

	//// Logger
	//logger = std::make_unique<Logger>();

	//if (!VERIFY(settings != nullptr, "Invalid argument"))
	//{
	//	return false;
	//}

	// game timer
	time = std::make_unique<Timer>(tickProvider);

	//// Event System
	//eventBus = std::make_unique<EventBus>();

	//// Window
	//window = std::make_unique<Window>();
	//if (!VERIFY(window->init(), "Failed to initialize window object"))
	//{
	//	return false;
	//}

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

	//// Renderer
	//renderer = std::make_unique<Renderer>();
	//renderer->setLogger(logger);
	//if (!VERIFY(renderer->init(), "Failed to initialize renderer"))
	//{
	//	return false;
	//}

	//// Input
	//input = std::make_unique<InputManager>(true);

	isInitialized = true;
	return isInitialized;
}

BRWL_NS_END
