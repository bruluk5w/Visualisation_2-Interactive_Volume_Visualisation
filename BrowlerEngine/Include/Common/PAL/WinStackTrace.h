#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_PAL_NS

class WinStackTrace
{

public:
    static const int maxTraceDepth = 60;

    static void recordStackTrace(WinStackTrace& stackTrace, unsigned int numSkipFrames = 0);

    void* stack[maxTraceDepth];
    struct FrameInfo
    {
        BRWL_CHAR moduleName[260];
        BRWL_CHAR functionName[260];
        BRWL_CHAR file[260];
        unsigned int line;
    } stackInfo[maxTraceDepth];
    unsigned short traceDepth;
};


BRWL_PAL_NS_END

#endif // BRWL_PLATFORM_WINDOWS