#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

#include "PAL/BrowlerWindowsInclude.h"

BRWL_PAL_NS

class WinStackTrace
{

public:
    static const int maxTraceDepth = 60;

    static void recordStackTrace(WinStackTrace& stackTrace, unsigned int numSkipFrames = 0);

    void* stack[maxTraceDepth];
    struct FrameInfo
    {
        BRWL_CHAR moduleName[MAX_PATH];
        BRWL_CHAR functionName[MAX_PATH];
        BRWL_CHAR file[MAX_PATH];
        unsigned int line;
    } stackInfo[maxTraceDepth];
    USHORT traceDepth;
};


BRWL_PAL_NS_END

#endif // BRWL_PLATFORM_WINDOWS