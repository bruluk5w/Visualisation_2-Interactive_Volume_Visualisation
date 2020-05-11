#pragma once

BRWL_NS


// this global functionpointer allows to set a function that can handle errors and possibly skip them
// return values:
//   - true: skip error
//   - false: break program in DEBUG or exit in RELEASE
extern bool(*globalExceptionHandler)(const BRWL_CHAR* test, const BRWL_CHAR* msg, bool mayIgnore);

void exception(const BRWL_CHAR* const test, const BRWL_CHAR* const msg, const bool mayIgnore);

BRWL_STR getExceptionString(const BRWL_CHAR* const test, const BRWL_CHAR* const  msg);

#define BRWL_EXCEPTION(test, msg) do { if (!(test)) BRWL::exception(BRWL_CHAR_LITERAL(#test), (msg), false);} while(0)
#define BRWL_VERIFY(test, msg) ([&] () -> bool { \
	if (!(test)) { \
		BRWL::exception(BRWL_CHAR_LITERAL(#test), (msg), true); \
		return false; \
	} else { \
		return true;\
	} \
})()

#ifdef _DEBUG

#define BRWL_CHECK(test, msg) do { \
static bool thrown = false; \
if (!thrown && !(test)) { \
BRWL::exception(BRWL_CHAR_LITERAL(#test), (msg), true); \
thrown = true; \
} \
} while(0)

#else
#define BRWL_CHECK
#endif

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

void printStackTrace(BRWL_STR& outStackPrint, unsigned int numSkipFrames = 0, bool printModuleName = false);

template<size_t maxMsgLen>
void printStackTrace(BRWL_CHAR (&output)[maxMsgLen], const BRWL_CHAR* test, const BRWL_CHAR* msg, unsigned int numSkipFrames = 0, bool printModuleName = false)
{
	BRWL_STR stackPrint;
	printStackTrace(stackPrint, 1 + numSkipFrames, printModuleName);
	const bool hasMsg = msg != nullptr;

	const BRWL_CHAR* msgFormat = hasMsg ? BRWL_CHAR_LITERAL("Exception:\n\"%s\"\nevalutated to FALSE:\n%s\n%s") : BRWL_CHAR_LITERAL("Exception:\n\"%s\"\nevalutated to FALSE!\n%s");
	size_t msgFormatAdditionalCharacters = stackPrint.size() + BRWL_STRLEN(msgFormat) - (hasMsg ? 3 * 2 : 2 * 2);

	size_t requiredArrayLen = BRWL_STRLEN(test) + (hasMsg ? BRWL_STRLEN(msg) : 0) + msgFormatAdditionalCharacters + 1;

	if (requiredArrayLen > maxMsgLen)
	{
		BRWL_SNPRINTF(output, BRWL::countof(output), msgFormat, test, hasMsg ? msg : stackPrint.c_str(), stackPrint.c_str());
	}
	else
	{
		// TODO: add text hint at the end that the message has been cut
		BRWL_SNPRINTF(output, BRWL::countof(output), msgFormat, test, hasMsg ? msg : stackPrint.c_str(), stackPrint.c_str());
	}
}

#define BRWL_UNREACHABLE() BRWL_EXCEPTION(false, BRWL_CHAR_LITERAL("Unreachable Code Reached!"))


BRWL_NS_END

BRWL_PAL_NS


#if defined(_DEBUG) && defined(BRWL_PLATFORM_WINDOWS)
// Only use this in debug configuration. We don't want this window to pop up in a release build.
void ShowLastWindowsError();
#endif


BRWL_PAL_NS_END