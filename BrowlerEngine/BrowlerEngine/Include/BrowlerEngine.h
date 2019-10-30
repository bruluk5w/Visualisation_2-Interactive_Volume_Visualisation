#pragma once

#include <mutex>
#include <array>

#include "main.h"

BRWL_NS


class Engine
{
	int data;
};


class MetaEngine 
{
	typedef uint8_t EngineHandle;

	MetaEngine(PlatformGlobals globals);

	void initialize();

	void run();

	bool CreateEngine(EngineHandle& handle);


	std::mutex metaEngineLock;
	
	std::array<std::unique_ptr<Engine>, 4> engines;

	EngineHandle currentEngine;

	static_assert(std::tuple_size<decltype(engines)>::value <= std::numeric_limits<EngineHandle>::max());

	PlatformGlobals globals;
};


BRWL_NS_END
