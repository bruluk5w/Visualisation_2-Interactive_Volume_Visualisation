#pragma once

BRWL_NS

// this global functionpointer allows to set a function that can handle errors and possibly skip them
// return values:
//   - true: skip error
//   - false: break program in DEBUG or exit in RELEASE
extern bool(*globalExceptionHandler)(const char* test, const char* msg, bool mayIgnore);

void exception(const char* const test, const char* const msg, const bool mayIgnore);

std::string getExceptionString(const char* const test, const char* const  msg);

#define BRWL_EXCEPTION(test, msg) do { if (!(test)) BRWL::exception(#test, (msg), false);} while(0)
#define BRWL_VERIFY(test, msg) ([&] () -> bool { \
	if (!(test)) { \
		BRWL::exception(#test, #msg, true); \
		return false; \
	} else { \
		return true;\
	} \
})()

struct ScopedNoDebugBreakOnException
{
	ScopedNoDebugBreakOnException();
	~ScopedNoDebugBreakOnException();

	bool previousValue;
};

struct ScopedIgnoreAllExceptions
{
	ScopedIgnoreAllExceptions();
	~ScopedIgnoreAllExceptions();

	bool previousValue;
};


BRWL_NS_END