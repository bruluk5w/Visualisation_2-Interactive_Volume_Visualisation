#pragma once

#include "Thread.h"

#include "Globals.h"
#include "EventSystem.h"
#include <mutex>

BRWL_NS


class Engine;
class Timer;
class TickProvider;
class Logger;


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

	MetaEngine(PlatformGlobalsPtr globals);
	~MetaEngine();

	void initialize();

	// Run a single step of each engine in which is not in DETACHED mode
	void update();
private:
	// Runs an independent loop for engines which are in DETACHED mode
	void detachedRun();

public:
	bool createEngine(EngineHandle& handle, const char* settingsFile = nullptr);
	void setEngineRunMode(EngineHandle handle, EngineRunMode runMode);
	EngineHandle getDefaultEngineHandle() { return defaultEngineHandle; }
	// Only use the returned pointer temporarily
	// Only use this 
	Engine* getEngine(EngineHandle handle);

	void shutDown();

private:
	bool checkHandle(EngineHandle handle, const BRWL_CHAR*& errorMsg);
	bool isInitialized;
	std::recursive_mutex metaEngineLock;
	EngineHandle defaultEngineHandle;
	
	struct EngineData {
		EngineData(std::unique_ptr<TickProvider>& tickProvider, PlatformGlobalsPtr globals, bool threaded);

		std::unique_ptr<TickProvider> tickProvider;
		std::unique_ptr<Engine> engine;
		bool threaded;

	};

	std::unique_ptr<EngineData> engines[maxEngine];
	std::unique_ptr<Thread<void>> frameThreads[maxEngine];
	
	PlatformGlobalsPtr globals;
};

class Engine
{
public:
	Engine(TickProvider* tickProvider, PlatformGlobals* globals);
	~Engine();

	bool init(const char* settingsFile);
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
	//std::unique_ptr<Window> window;
	//std::unique_ptr<Renderer> renderer;
	//std::unique_ptr<InputManager> input;
	//std::unique_ptr<Hierarchy> hierarchy;
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
};

BRWL_NS_END
