#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_PAL_NS
class WinGlobals;
class ReadOnlyWinGlobals;
BRWL_PAL_NS_END

BRWL_NS
typedef  PAL::ReadOnlyWinGlobals PlatformGlobals;
BRWL_NS_END

#endif
