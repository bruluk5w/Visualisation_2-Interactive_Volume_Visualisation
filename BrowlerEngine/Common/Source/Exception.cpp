#include "Exception.h"

#include <iostream>

BRWL_NS


namespace
{
	thread_local bool doDebugBreakOnException = true;
	thread_local bool ignoreAllExceptions = false;
}

bool(*globalExceptionHandler)(const char* test, const char* msg, bool mayIgnore) = nullptr;

std::string getExceptionString(const char* const test, const char* const msg)
{
	constexpr char const prefix[] = "EXCEPTION: \n(";
	constexpr char const postfix[] = ")\n= false.\nMessage:\n";
	std::string outMsg;
	outMsg.reserve(sizeof(prefix) + sizeof(postfix) - 1 + strlen(test) + strlen(msg));
	return outMsg.append(prefix).append(test).append(postfix).append(msg);
}

void exception(const char* const test, const char* const msg, const bool mayIgnore)
{
	if (ignoreAllExceptions)
	{
		std::cout << msg << std::endl;
		return;
	}
	if (!globalExceptionHandler || !globalExceptionHandler(test, msg, mayIgnore) || !mayIgnore)
	{
#ifdef _DEBUG
		if (doDebugBreakOnException)
		{
			__debugbreak();
		}
#else
		if (doDebugBreakOnException || !mayIgnore)
		{
			throw std::runtime_error(getExceptionString(test, msg));
		}
#endif
	}
}

ScopedNoDebugBreakOnException::ScopedNoDebugBreakOnException()
{
	previousValue = doDebugBreakOnException;
	doDebugBreakOnException = false;
}

ScopedNoDebugBreakOnException::~ScopedNoDebugBreakOnException()
{
	doDebugBreakOnException = previousValue;
}

ScopedIgnoreAllExceptions::ScopedIgnoreAllExceptions()
{
	previousValue = ignoreAllExceptions;
	ignoreAllExceptions = true;
}

ScopedIgnoreAllExceptions::~ScopedIgnoreAllExceptions()
{
	ignoreAllExceptions = previousValue;
}


BRWL_NS_END