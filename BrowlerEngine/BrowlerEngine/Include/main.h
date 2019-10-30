#pragma once

#include "PAL/WinMain.h"

BRWL_NS

#ifdef BRWL_PLATFORM_WINDOWS
typedef PAL::WinGlobals PlatformGlobals;
#endif

BRWL_NS_END