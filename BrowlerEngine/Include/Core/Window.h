#pragma once

#include "PAL/WinWindow.h"

BRWL_NS


#ifdef BRWL_PLATFORM_WINDOWS
#include "PAL/WinWindow.h"
	typedef PAL::WinWindow PlatformWindow;
#endif


BRWL_NS_END