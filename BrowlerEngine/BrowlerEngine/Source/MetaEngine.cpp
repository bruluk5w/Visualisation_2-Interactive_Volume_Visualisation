#include "BrowlerEngine.h"

#include "Timer.h"

BRWL_NS


// Global pointer to the currently running engine
thread_local Engine* engine;

MetaEngine::MetaEngine(PlatformGlobals* globals) :
	isInitialized(false),
	metaEngineLock(),
	tickProvider(nullptr),
	defaultEngineHandle(maxEngine),
	engines{ nullptr },
	frameThreads{ nullptr },
	globals(globals)
{ }

void MetaEngine::initialize()
{
	std::lock_guard<std::mutex> guard(metaEngineLock);

	// create one default engine
	if (*engines == nullptr) createEngine(defaultEngineHandle);
	if (tickProvider == nullptr) tickProvider = std::make_unique<TickProvider>();
	isInitialized = true;
}

void MetaEngine::update()
{
	BRWL_EXCEPTION(isInitialized, "\"initialize\" has to be called before \"update\"!");
	std::lock_guard<std::mutex> guard(metaEngineLock);

	// here run the main loops of the single engines in parallel threads 
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		EngineRunMode mode = engines[i]->getRunMode();
		if (mode == EngineRunMode::SYNCHRONIZED)
		{
			if (frameThreads[i] == nullptr)
			{
				frameThreads[i] = std::make_unique<Thread<void>>([this, i] {this->engines[i]->update(); }, nullptr);
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
		if (engines[i]->getRunMode() == EngineRunMode::META_ENGINE_MAIN_THREAD) {
			engines[i]->update();
		}
	}

	// join the previously started parallel threads
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		if (engines[i]->getRunMode() == EngineRunMode::SYNCHRONIZED)
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
	std::lock_guard<std::mutex> guard(metaEngineLock);
	std::unique_ptr<Engine>* nextEnginePtr = std::find(engines, engines + countof(engines), nullptr);
	if (!BRWL_VERIFY(nextEnginePtr < engines + countof(engines), "Max number of engines exceeded!"))
	{
		handle = maxEngine;
		return false;
	}

	*nextEnginePtr = std::make_unique<Engine>(tickProvider.get(), globals);
	(*nextEnginePtr)->init(settingsFile);

	handle = (uint8_t)(nextEnginePtr - engines);

	return true;
}

void MetaEngine::SetEngineRunMode(EngineHandle handle, EngineRunMode runMode)
{
	BRWL_EXCEPTION(handle < maxEngine, "Invalid engine handle");
	BRWL_EXCEPTION(engines[handle] != nullptr, "Invalid engine handle");
	engines[handle]->writeAccessMetaEngine().setRunMode(runMode);
}


BRWL_NS_END