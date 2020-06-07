#include "PAL/WinStackTrace.h"

#ifdef BRWL_USE_DEBUG_SYMBOLS
#include "dbghelp.h"
#endif


BRWL_PAL_NS


void WinStackTrace::recordStackTrace(WinStackTrace& stackTrace, unsigned int numSkipFrames) {
    memset(&stackTrace, 0, sizeof(stackTrace));

    stackTrace.traceDepth = CaptureStackBackTrace(
        1UL + numSkipFrames,    // ULONG  FramesToSkip
        (ULONG)maxTraceDepth,   // ULONG  FramesToCapture
        stackTrace.stack,       // PVOID * BackTrace
        nullptr                 // PULONG BackTraceHash
    );

#ifdef BRWL_USE_DEBUG_SYMBOLS
    HANDLE process = GetCurrentProcess();
    static_assert(INVALID_HANDLE_VALUE == (HANDLE)-1);
    for (int i = 0; i < stackTrace.traceDepth; ++i)
    {

#ifdef _WIN64
        DWORD64 addr = (DWORD64)stackTrace.stack[i];
        DWORD64 moduleBase = SymGetModuleBase(process, addr);
#else 
        DWORD addr = (DWORD)stackTrace.stack[i];
        DWORD moduleBase = SymGetModuleBase(process, addr);
#endif

        FrameInfo& frame = stackTrace.stackInfo[i];
        if (moduleBase)
        {
#ifdef UNICODE
            {
                const decltype(frame.moduleName) dummy = {}; // is only here to derive the size :/
                char moduleName[countof(dummy)] = { 0 };

                DWORD ret = GetModuleFileNameA((HINSTANCE)moduleBase, moduleName, sizeof(moduleName));
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, frame.moduleName, moduleName, countof(moduleName));
            }
#else
            GetModuleFileNameA((HINSTANCE)moduleBase, frame.moduleName, sizeof(frame.moduleName));
#endif
        }

        const decltype(frame.functionName) dummy = {}; // is only here to derive the size :/
        char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + countof(dummy)];
        PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
        symbol->SizeOfStruct = sizeof(symbolBuffer);
        symbol->MaxNameLength = (decltype(symbol->MaxNameLength))countof(dummy);

        if (SymGetSymFromAddr(process, addr, NULL, symbol))
        {
#ifdef UNICODE
            {
                char functionName[countof(dummy)] = { 0 };
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, frame.functionName, symbol->Name, countof(dummy));
            }
#else
            strcpy_s(frame.functionName, countof(frame.functionName), symbol->Name);
#endif
        }

        DWORD  offset = 0;
        IMAGEHLP_LINE line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        if (SymGetLineFromAddr(process, addr, &offset, &line))
        {

#ifdef UNICODE
            {
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, frame.file, line.FileName, sizeof(frame.file));
            }
#else
            strcpy_s(frame.file, countof(frame.file), line.FileName);
#endif
            frame.line = line.LineNumber;
        }
    }

#endif

}


BRWL_PAL_NS_END