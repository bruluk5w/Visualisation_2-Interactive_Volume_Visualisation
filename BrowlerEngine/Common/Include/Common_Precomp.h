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

#pragma warning(error:4003) // not enough actual parameters for macro
#pragma warning(error:4715) // not all control paths return a value

#include "Utilities.h"
#include "Exception.h"
