#include "BrowlerEngine.h"

#include "Timer.h"

BRWL_NS


// Global pointer to the currently running engine
thread_local Engine* engine;

MetaEngine::MetaEngine(PlatformGlobalsPtr globals) :
	isInitialized(false),
	metaEngineLock(),
	defaultEngineHandle(maxEngine),
	engines{ nullptr },
	frameThreads{ nullptr },
	globals(globals)
{ }

MetaEngine::~MetaEngine()
{
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		if (frameThreads[i] != nullptr)
		{
			BRWL_EXCEPTION(frameThreads[i]->getState() != ThreadState::RUNNING, "MetaEngine delte when engines are still running!");
		}
	}
}

void MetaEngine::initialize()
{
	std::lock_guard<decltype(metaEngineLock)> guard(metaEngineLock);

	isInitialized = true;

	// create one default engine
	if (*engines == nullptr) createEngine(defaultEngineHandle);
}

void MetaEngine::shutDown() {
	//Todo: wait for all frames to end remove all engines
	// Watch out for deadlocks
}

void MetaEngine::update()
{
	BRWL_EXCEPTION(isInitialized, "\"initialize\" has to be called before \"update\"!");
	std::lock_guard<decltype(metaEngineLock)> guard(metaEngineLock);
	// here run the main loops of the single engines in parallel threads 
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		EngineRunMode mode = engines[i]->engine->getRunMode();
		if (mode == EngineRunMode::SYNCHRONIZED)
		{
			if (frameThreads[i] == nullptr)
			{
				frameThreads[i] = std::make_unique<Thread<void>>([this, i] {
					engine = engines[i]->engine.get();  // set (per thread) global pointer to currently running engine
					engine->update();
					engine = nullptr;
				}, nullptr);
			}

			BRWL_EXCEPTION(frameThreads[i]->getState() == ThreadState::READY, "Something is fishy, the thread is RUNNING but it should be set to READY");

			bool successDispatch = frameThreads[i]->run();
			BRWL_EXCEPTION(successDispatch, "Failed to start frame thread!");
		}
		else if (mode == EngineRunMode::DETATCHED)
		{
			// TODO: check detached threads whether some are new and have to kicked off
		}
	}

	// run those threads sequentially which want to run on the main thread
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		if (engines[i]->engine->getRunMode() == EngineRunMode::META_ENGINE_MAIN_THREAD)
		{
			engine = engines[i]->engine.get();
			engine->update();
			engine = nullptr;
		}
	}

	// join the previously started parallel threads
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		if (engines[i]->engine->getRunMode() == EngineRunMode::SYNCHRONIZED)
		{
			frameThreads[i]->join();
			frameThreads[i]->rewind();
		}
	}
}

void MetaEngine::detachedRun()
{

}

bool MetaEngine::createEngine(EngineHandle& handle, const char* settingsFile/* = nullptr*/)
{
	BRWL_EXCEPTION(isInitialized, "\"initialize\" has to be called before \"createEngine\"!");
	std::lock_guard<decltype(metaEngineLock)> guard(metaEngineLock);
	std::unique_ptr<EngineData>* nextEnginePtr = std::find(engines, engines + countof(engines), nullptr);
	if (!BRWL_VERIFY(nextEnginePtr < engines + countof(engines), "Max number of engines exceeded!"))
	{
		handle = maxEngine;
		return false;
	}

	std::unique_ptr<TickProvider> tickProvider = std::make_unique<TickProvider>();

	*nextEnginePtr = std::make_unique<EngineData>(tickProvider,  globals, false);
	(*nextEnginePtr)->engine->init(settingsFile);

	handle = (uint8_t)(nextEnginePtr - engines);

	return true;
}

void MetaEngine::setEngineRunMode(EngineHandle handle, EngineRunMode runMode)
{
	BRWL_EXCEPTION(handle < maxEngine, "Invalid engine handle");
	BRWL_EXCEPTION(engines[handle] != nullptr, "Invalid engine handle");
	engines[handle]->engine->writeAccessMetaEngine().setRunMode(runMode);
}

MetaEngine::EngineData::EngineData(std::unique_ptr<TickProvider>& tickProvider, PlatformGlobalsPtr globals, bool threaded) :
	tickProvider(std::move(tickProvider)),
	engine(std::make_unique<Engine>(this->tickProvider.get(), globals)),
	threaded(threaded)
{ }


BRWL_NS_END