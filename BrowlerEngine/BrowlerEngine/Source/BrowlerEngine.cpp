#include "BrowlerEngine.h"

BRWL_NS

MetaEngine::MetaEngine(PlatformGlobals globals) :
	globals(globals),
	engines(),
	currentEngine(0),
	nextEngine(1)
{

}

void MetaEngine::initialize()
{
	// initialize stuff like parsing command line etc.
}

void MetaEngine::run()
{
	// here run the main loops of the single engines
}

bool MetaEngine::CreateEngine(EngineHandle& handle)
{
	std::lock_guard<std::mutex> lock(metaEngineLock);
	if (nextEngine + 1 > std::tuple_size<decltype(engines)>::value) {
		return false;
	}

	engines[currentEngine + 1] = std::make_unique<Engine>();
	handle = ++currentEngine;
	return true;
}

BRWL_NS_END
