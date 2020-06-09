#pragma once // (c) 2020 Lukas Brunner

#include "Thread.h"

#include "EventSystem.h"
#include "Common/GlobalsFwd.h"
#include "WindowFwd.h"
#include "Renderer/RendererFwd.h"
#include <mutex>

BRWL_NS


class Engine;
class Timer;
class TickProvider;
class Logger;
class Hierarchy;
namespace RENDERER{
	class Camera;
}

class MetaEngine final
{
public:
	typedef uint8_t EngineHandle;
	static const EngineHandle maxEngine = 4;
	enum class EngineRunMode : uint8_t
	{
		META_ENGINE_MAIN_THREAD = 0,
		SYNCHRONIZED,
		DETATCHED,
	};

	MetaEngine(PlatformGlobals* globals);
	~MetaEngine();

	void initialize();

	// Run a single step of each engine in which is not in DETACHED mode
	void update();
private:
	// Runs an independent loop for engines which are in DETACHED mode
	struct EngineData;
	void detachedRun(EngineData* engine);

public:
	bool createEngine(EngineHandle& handle, const char* settingsFile = nullptr);
	void setEngineRunMode(EngineHandle handle, EngineRunMode runMode);
	EngineHandle getDefaultEngineHandle() { return defaultEngineHandle; }
	// Only use the returned pointer temporarily
	// Only use this 
	Engine* getEngine(EngineHandle handle);

	void shutDown();
protected:
	bool checkHandle(EngineHandle handle, const BRWL_CHAR*& errorMsg);

private:
	bool isInitialized;
	std::recursive_mutex metaEngineLock;
	EngineHandle defaultEngineHandle;
	
	struct EngineData {
		EngineData(std::unique_ptr<TickProvider>& tickProvider, PlatformGlobals* globals, bool threaded);

		std::unique_ptr<TickProvider> tickProvider;
		std::unique_ptr<Engine> engine;
		bool threaded;
	};

	std::unique_ptr<EngineData> engines[maxEngine];
	std::unique_ptr<Thread<void>> frameThreads[maxEngine];
	
	PlatformGlobals* globals;
};

class Engine
{
public:
	Engine(TickProvider* tickProvider, PlatformGlobals* globals);
	~Engine();
	// Called from the thread which is initally creating the Engine
	bool init(const char* settingsFile);
	// Called from the thread which will also subsequently call the update method
	void threadInit();
	// Called from the thread which called threadInit
	void threadDestroy();
	// Returns true if "init" and "threadInit" succeeded and "close" has not yet been called
	bool IsInitialized() const { return isInitialized; }
	void update();
	void shutdown();
	bool shouldClose();
	// this one should acutally only be called by the main function
	void close();



	// convenience functions for logging
	void LogDebug(const BRWL_CHAR* msg);
	void LogInfo(const BRWL_CHAR* msg);
	void LogWarning(const BRWL_CHAR* msg);
	void LogError(const BRWL_CHAR* msg);

	//std::shared_ptr<Settings> settings;
	std::shared_ptr<Logger> logger;
	std::unique_ptr<Timer> time;
	std::unique_ptr<CoreEventSystem> eventSystem;
	std::unique_ptr<Window> window;
	std::unique_ptr<RENDERER::Renderer> renderer;
	std::unique_ptr<Hierarchy> hierarchy;
	//std::unique_ptr<InputManager> input;
	//std::unique_ptr<MeshRegistry> meshRegistry;
	//std::unique_ptr<TextureRegistry> textureRegistry;

	MetaEngine::EngineRunMode getRunMode() const { return runMode; }

	struct WriteAccessMetaEngine {
		friend class MetaEngine;
		Engine* pThis;
	private:
		void setRunMode(MetaEngine::EngineRunMode mode) { pThis->runMode = mode; }
	};

	WriteAccessMetaEngine writeAccessMetaEngine() { return  { this }; }

protected:
	bool internalInit(const char* settingsFile);
	
	bool isInitialized;
	TickProvider* tickProvider;
	PlatformGlobals* globals;
	MetaEngine::EngineRunMode runMode;
	std::unique_ptr<RENDERER::Camera> defaultCamera;
};

BRWL_NS_END
