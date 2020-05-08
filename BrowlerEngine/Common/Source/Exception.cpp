#include "Exception.h"

#include <iostream>

#include "StackTrace.h"

BRWL_NS


namespace
{
	thread_local bool doDebugBreakOnException = true;
	thread_local bool ignoreAllExceptions = false;
}

bool(*globalExceptionHandler)(const BRWL_CHAR* test, const BRWL_CHAR* msg, bool mayIgnore) = nullptr;

BRWL_STR getExceptionString(const BRWL_CHAR* const test, const BRWL_CHAR* const msg)
{
	const BRWL_CHAR prefix[] = BRWL_CHAR_LITERAL("EXCEPTION: \n");
	const BRWL_CHAR postfix[] = BRWL_CHAR_LITERAL("\n= false.\nMessage:\n");
	BRWL_STR outMsg;
	outMsg.reserve(countof(prefix) + countof(postfix) + BRWL_STRLEN(test) + (msg != nullptr ? BRWL_STRLEN(msg) : 0) - 1);
	outMsg.append(prefix).append(test).append(postfix);
	if (msg != nullptr)
	{
		outMsg.append(msg);
	}

	return outMsg;
}

void exception(const BRWL_CHAR* const test, const BRWL_CHAR* const msg, const bool mayIgnore)
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
			
#ifdef UNICODE
			BRWL_STR mbStr = getExceptionString(test, msg);
			// warning C4244 : 'argument' : conversion from 'wchar_t' to 'const _Elem', possible loss of data
			// for the sake of simplicity we want a truncating conversion here
#pragma warning( push )
#pragma warning( disable : 4244)
			std::string str (mbStr.begin(), mbStr.end());
#pragma warning( pop ) 
#else
			std::string str = getExceptionString(test, msg);
#endif // UNICODE

			throw std::runtime_error(str);
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

void printStackTrace(BRWL_STR& outStackPrint, unsigned int numSkipFrames /*= 0*/, bool printModuleName /*= false*/)
{
	printModuleName = true;
	std::unique_ptr<StackTrace> stackTrace = std::make_unique<BRWL::StackTrace>();
	StackTrace::recordStackTrace(*stackTrace, /*numSkipFrames=*/1 + numSkipFrames);

	const BRWL_CHAR* stackFrameFormat = printModuleName ? BRWL_CHAR_LITERAL("%s!%s, %s, Line %u\n") : BRWL_CHAR_LITERAL("%s!%s, Line %u\n");
	const size_t additionalCharsPerStackFrame = (printModuleName ? 20 - 4 * 2 : 16 - 3 * 2) - 1;

	size_t msgFormatAdditionalCharacters = stackTrace->traceDepth * additionalCharsPerStackFrame;
	for (int i = 0; i < stackTrace->traceDepth; ++i)
	{
		msgFormatAdditionalCharacters += BRWL_STRLEN(stackTrace->stackInfo[i].moduleName);
		msgFormatAdditionalCharacters += BRWL_STRLEN(stackTrace->stackInfo[i].functionName);
		msgFormatAdditionalCharacters += BRWL_STRLEN(stackTrace->stackInfo[i].file);
		if (stackTrace->stackInfo[i].line > 0) msgFormatAdditionalCharacters += (size_t)std::ceil(std::log10((float)stackTrace->stackInfo[i].line));
	}

	outStackPrint.clear();
	outStackPrint.resize(msgFormatAdditionalCharacters);
	int endIdx = 0;
	for (int i = 0; i < stackTrace->traceDepth; ++i)
	{
		int numWritten = BRWL_SNPRINTF(
			&outStackPrint[endIdx], msgFormatAdditionalCharacters - endIdx, stackFrameFormat,
			stackTrace->stackInfo[i].moduleName, stackTrace->stackInfo[i].functionName, stackTrace->stackInfo[i].file, stackTrace->stackInfo[i].line
		);

		if (numWritten > 0)
		{
			endIdx += numWritten;
		}
		else
		{
			break;
		}
	}
}


BRWL_NS_END

BRWL_PAL_NS


#if defined(_DEBUG) && defined(BRWL_PLATFORM_WINDOWS)
#include <strsafe.h>
void ShowLastWindowsError()
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("failed with error %d: %s"),
		dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
#endif


BRWL_PAL_NS_END