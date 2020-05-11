#pragma once

#include "PAL/WinGlobals.h"


BRWL_NS


#ifdef BRWL_PLATFORM_WINDOWS
typedef PAL::ReadOnlyWinGlobals PlatformGlobals;
typedef PAL::ReadOnlyWinGlobals* PlatformGlobalsPtr;
#endif


BRWL_NS_END