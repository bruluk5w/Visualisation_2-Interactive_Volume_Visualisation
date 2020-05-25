#include "BrowlerEngine.h"

#include "Timer.h"

BRWL_NS

// Global pointer to the currently running engine
thread_local Engine* engine;

namespace {
	const BRWL_CHAR* handleOutOfBoundsMsg = BRWL_CHAR_LITERAL("Engine handle out of bounds.");
	const BRWL_CHAR* invalidHandleMsg = BRWL_CHAR_LITERAL("No Engine exists for this handle.");
}

bool MetaEngine::checkHandle(EngineHandle handle, const BRWL_CHAR*& errorMsg)
{
	if (handle >= maxEngine) {
		errorMsg = handleOutOfBoundsMsg;
		return false;
	} else if (engines[handle] == nullptr) {
		errorMsg = invalidHandleMsg;
		return false;
	}

	errorMsg = nullptr;
	return true;
}

Engine* MetaEngine::getEngine(EngineHandle handle)
{
	const BRWL_CHAR* msg;
	BRWL_EXCEPTION(checkHandle(handle, msg), msg);
	return engines[handle]->engine.get();
}

MetaEngine::MetaEngine(PlatformGlobals* globals) :
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
			BRWL_EXCEPTION(frameThreads[i]->getState() != ThreadState::RUNNING, BRWL_CHAR_LITERAL("MetaEngine delete when engines are still running!"));
		}
	}
}

void MetaEngine::initialize()
{
	std::lock_guard<decltype(metaEngineLock)> guard(metaEngineLock);

	isInitialized = true;

	// create one default engine
	if (*engines == nullptr) {
		bool success = createEngine(defaultEngineHandle);
		BRWL_EXCEPTION(success, BRWL_CHAR_LITERAL("Enginge creation failed!"));
	}
}

void MetaEngine::shutDown() {
	//Todo: wait for all frames to end remove all engines
	// Watch out for deadlocks
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		std::unique_ptr<EngineData>& engineData = engines[i];
		std::unique_ptr<Thread<void>>& thread = frameThreads[i];
		EngineRunMode mode = engineData->engine->getRunMode();
		BRWL_EXCEPTION(mode != EngineRunMode::DETATCHED || (thread->getState() == ThreadState::READY || thread->getState() == ThreadState::TERMINATED),
			BRWL_CHAR_LITERAL("Invalid thread state on shutdown"));

		if (mode == EngineRunMode::DETATCHED)
		{
			engineData->threaded = false; // signal termination to thread
		}
	}

	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		const EngineData* const engineData = engines[i].get();
		EngineRunMode mode = engineData->engine->getRunMode();
		if (mode == EngineRunMode::DETATCHED)
		{
			Thread<void>* const thread = frameThreads[i].get();
			thread->join();
			thread->rewind();
		}
	}
}

void MetaEngine::update()
{
	BRWL_EXCEPTION(isInitialized, BRWL_CHAR_LITERAL("\"initialize\" has to be called before \"update\"!"));
	std::lock_guard<decltype(metaEngineLock)> guard(metaEngineLock);
	// here run the main loops of the single engines in parallel threads 
	for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	{
		std::unique_ptr<Thread<void>>& thread = frameThreads[i];
		std::unique_ptr<EngineData>& engineData = engines[i];
		EngineRunMode mode = engineData->engine->getRunMode();
		BRWL_EXCEPTION(mode == EngineRunMode::DETATCHED, BRWL_CHAR_LITERAL("Only threaded mode suppoerted."));
		if (mode == EngineRunMode::SYNCHRONIZED)
		{
			//if (thread == nullptr)
			//{
			//	thread = std::make_unique<Thread<void>>([this, &engineData] {
			//		engine = engineData->engine.get();  // set (per thread) global pointer to currently running engine
			//		if (!engine->IsInitialized())
			//		{
			//			engine->threadInit();
			//		}

			//		engineData->tickProvider->nextFrame();
			//		engine->update();
			//		engine = nullptr;
			//	}, nullptr);
			//}
		}
		else if (mode == EngineRunMode::DETATCHED)
		{
			if (thread == nullptr)
			{
				engineData->threaded = true;
				thread = std::make_unique<Thread<void>>(std::bind(&MetaEngine::detachedRun, this, engineData.get()), nullptr);
			}
			else if (thread->getState() == ThreadState::RUNNING)
			{
				continue;
			}
		}

		if (mode != EngineRunMode::META_ENGINE_MAIN_THREAD)
		{
			BRWL_EXCEPTION(frameThreads[i]->getState() == ThreadState::READY, BRWL_CHAR_LITERAL("Something is fishy, the thread is RUNNING but it should be set to READY"));

			bool successDispatch = frameThreads[i]->run();
			BRWL_EXCEPTION(successDispatch, BRWL_CHAR_LITERAL("Failed to start frame thread!"));
		}
	}

	//// run those threads sequentially which want to run on the main thread
	//for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	//{
	//	if (engines[i]->engine->getRunMode() == EngineRunMode::META_ENGINE_MAIN_THREAD)
	//	{
	//		engine = engines[i]->engine.get();
	//		engines[i]->tickProvider->nextFrame();
	//		engine->update();
	//		engine = nullptr;
	//	}
	//}

	//// join the previously started parallel threads
	//for (unsigned int i = 0; i < countof(engines) && engines[i] != nullptr; ++i)
	//{
	//	if (engines[i]->engine->getRunMode() == EngineRunMode::SYNCHRONIZED)
	//	{
	//		frameThreads[i]->join();
	//		frameThreads[i]->rewind();
	//	}
	//}
}

void MetaEngine::detachedRun(EngineData* engineData)
{
	engine = engineData->engine.get();  // set (per thread) global pointer to currently running engine

	if (!engine->IsInitialized())
	{
		engine->threadInit();
	}

	while (engineData->threaded)
	{
		Sleep(10);
		engineData->tickProvider->nextFrame();
		engineData->engine->update();
	}

	engine->threadDestroy();

	engine = nullptr;
}

bool MetaEngine::createEngine(EngineHandle& handle, const char* settingsFile/* = nullptr*/)
{
	BRWL_EXCEPTION(isInitialized, BRWL_CHAR_LITERAL("\"initialize\" has to be called before \"createEngine\"!"));
	std::lock_guard<decltype(metaEngineLock)> guard(metaEngineLock);
	std::unique_ptr<EngineData>* nextEnginePtr = std::find(engines, engines + countof(engines), nullptr);
	if (!BRWL_VERIFY(nextEnginePtr < engines + countof(engines), BRWL_CHAR_LITERAL("Max number of engines exceeded!")))
	{
		handle = maxEngine;
		return false;
	}

	std::unique_ptr<TickProvider> tickProvider = std::make_unique<TickProvider>();

	*nextEnginePtr = std::make_unique<EngineData>(tickProvider,  globals, false);
	if (!BRWL_VERIFY((*nextEnginePtr)->engine->init(settingsFile), BRWL_CHAR_LITERAL("Failed to initialize engine!")))
	{
		nextEnginePtr = nullptr;
		tickProvider = nullptr;
		handle = maxEngine;
		return false;
	}

	handle = (uint8_t)(nextEnginePtr - engines);
	return true;
}

void MetaEngine::setEngineRunMode(EngineHandle handle, EngineRunMode runMode)
{
	const BRWL_CHAR* msg;
 	BRWL_EXCEPTION(checkHandle(handle, msg), msg);
	engines[handle]->engine->writeAccessMetaEngine().setRunMode(runMode);
}

MetaEngine::EngineData::EngineData(std::unique_ptr<TickProvider>& tickProvider, PlatformGlobals* globals, bool threaded) :
	tickProvider(std::move(tickProvider)),
	engine(std::make_unique<Engine>(this->tickProvider.get(), globals)),
	threaded(threaded)
{ }


BRWL_NS_END