#include "BrowlerEngine.h"

#include "Timer.h"
#include "Window.h"
#include "Renderer/Renderer.h"
#include "Input.h"
#include "Hierarchy.h"
#include "Renderer/Camera.h"


BRWL_NS


Engine::Engine(TickProvider* tickProvider, PlatformGlobals* globals) :
	logger(nullptr),
	time(nullptr),
	eventSystem(nullptr),
	window(nullptr),
	renderer(nullptr),
	initialized(false),
	tickProvider(tickProvider),
	globals(globals),
	runMode(MetaEngine::EngineRunMode::DETATCHED),
	updatables()
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
	window->create(100, 100, 1280, 720);
	window->setRenderer(renderer.get());
	initialized = true;
}

void Engine::threadDestroy()
{
	initialized = false;
	if (window)
	{
		window->setRenderer(nullptr);
		window->destroy();
	}
}

void Engine::update()
{
	FrameIdxChange param{ time->getFrameCount() };
	eventSystem->postEvent<Event::FRAME_IDX_CHANGE>(&param);

	input->preMessageUpdate();

	if (window && runMode == MetaEngine::EngineRunMode::DETATCHED)
	{	// only process messages if we are not receiving them from the parent window
		window->processPlatformMessages();
	}

	input->postMessageUpdate();

	{
		double dt = time->getDeltaTime();
		std::scoped_lock(updatablesMutex);
		std::for_each(updatables.begin(), updatables.end(), [dt](std::unique_ptr<IUpdatable>& updatable){
			if (updatable) {
				if (!updatable->isInitialized()) {
					updatable->internalInit();
				}
				else
				{	// wait one frame, then update
					updatable->update(dt);
				}

			}
		});
	}

	if (renderer && renderer->isInitialized()) {
		if (renderer->getCamera() == nullptr)
		{
			unsigned int width, height;
			renderer->getFrameBufferSize(width, height);
			defaultCamera = std::make_unique<RENDERER::Camera>((int)width, (int)height, 75.0f * DEG_2_RAD_F, 0.1f, 500.0f, BRWL_CHAR_LITERAL("Default Camera"));
			defaultCamera->position() = Vec3(0, 0, 0);
			hierarchy->addToRoot(defaultCamera.get());
			renderer->setCamera(defaultCamera.get());
		}
	}
	
	hierarchy->update();

	if (renderer && renderer->isInitialized())
	{
		renderer->preRender();
		renderer->render();
		renderer->draw();
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
	initialized = false;
	
	{
		std::scoped_lock(updatablesMutex);
		std::for_each(updatables.begin(), updatables.end(), [](std::unique_ptr<IUpdatable>& updatable) {updatable->internalDestroy(); });
		updatables.clear();
	}

	if (time) time->stop();

	// Input
	if (input)
	{
		input = nullptr;
	}

	// Default Camera
	if (defaultCamera)
	{
		defaultCamera = nullptr;
	}

	// Hierarchy
	hierarchy = nullptr;

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
		if (renderer->getCamera() == defaultCamera.get())
			renderer->setCamera(nullptr);
		renderer = nullptr;
	}

	// Window
	if (window)
	{
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
	renderer = std::make_unique<RENDERER::Renderer>(eventSystem.get(), globals);
	renderer->setLogger(logger);

	
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

	// Hierarchy
	hierarchy = std::make_unique<Hierarchy>();

	// Input
	input = std::make_unique<InputManager>();

	// Updatables
	{
		std::scoped_lock(updatablesMutex);
		updatables.clear();
		updatables.reserve(10);
	}
	return true;
}

void Engine::LogDebug(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->debug(msg); }
void Engine::LogInfo(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->info(msg); }
void Engine::LogWarning(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->warning(msg); }
void Engine::LogError(const BRWL_CHAR* msg) { if (BRWL_VERIFY(logger, BRWL_CHAR_LITERAL("Logger not created yet."))) logger->error(msg); }


Vec3 screenSpaceToWorldRay(float screenX, float screenY, RENDERER::Camera& camera) {
	Vec3 normalizedMouse = Vec3((screenX / engine->window->safeWidth() - 0.5f), (0.5f - screenY / engine->window->safeHeight()), 1.f);
	const Mat4& inv = camera.getInverseViewProjectionMatrix();
	Vec3 worldRay = normalizedMouse * inv;
	normalize(worldRay);
	worldRay.z *= -0.5f;
	return worldRay;
}

BRWL_NS_END
