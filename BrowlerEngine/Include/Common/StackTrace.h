#pragma once

#include "PAL/WinStackTrace.h"

BRWL_NS

#ifdef BRWL_PLATFORM_WINDOWS

typedef PAL::WinStackTrace StackTrace;

#endif



BRWL_NS_END