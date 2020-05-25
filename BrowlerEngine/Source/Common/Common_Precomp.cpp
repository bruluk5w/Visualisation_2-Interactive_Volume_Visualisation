#include "Common_Precomp.h"

#include <clocale>

#ifdef BRWL_USE_DEBUG_SYMBOLS
#include "dbghelp.h"
#endif

BRWL_NS

void earlyStaticInit()
{  
    ::std::setlocale(LC_ALL, "en_US.utf8");
#ifdef BRWL_USE_DEBUG_SYMBOLS
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES);
#endif
}

void lateStaticDestroy() {
    #ifdef BRWL_USE_DEBUG_SYMBOLS
    HANDLE process = GetCurrentProcess();
    SymCleanup(process);
    #endif
}

BRWL_NS_END