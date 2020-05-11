#define BRWL_NS namespace BRWL {
	
#define BRWL_NS_END }

#define BRWL_PAL_NS BRWL_NS \
namespace PAL {

#define BRWL_PAL_NS_END BRWL_NS_END\
}



#if defined(_WIN32) && !defined(BRWL_PLATFORM_WINDOWS)
#define BRWL_PLATFORM_WINDOWS
#endif


#include <cstdint>
#include <memory>
#include <string>
#include <cmath>

#pragma warning(error:4003) // not enough actual parameters for macro
#pragma warning(error:4715) // not all control paths return a value

#ifdef UNICODE
#define BRWL_CHAR wchar_t
//TODO: rename to something shorter 
#define BRWL_CHAR_LITERAL(LITERAL) L##LITERAL
#define BRWL_STRLEN wcslen
#define BRWL_STR ::std::wstring
#define BRWL_SNPRINTF swprintf
#else
#define BRWL_CHAR char
#define BRWL_CHAR_LITERAL(LITERAL) LITERAL
#define BRWL_STRLEN strlen
#define BRWL_STR ::std::string
#define BRWL_SNPRINTF snprintf

#endif

#ifdef _DEBUG
#define BRWL_USE_DEBUG_SYMBOLS
#endif

#include "Utilities.h"
#include "Exception.h"

BRWL_NS

// this function should be called as early as possible during startup of the program
void earlyStaticInit();
// this function should be called as late as possible during destruction of the program
void lateStaticDestroy();

BRWL_NS_END
