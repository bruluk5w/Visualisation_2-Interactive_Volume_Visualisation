#pragma once

BRWL_PAL_NS


#ifdef BRWL_PLATFORM_WINDOWS
class WinGlobals;
class ReadOnlyWinGlobals;
#endif


BRWL_PAL_NS_END


BRWL_NS


#ifdef BRWL_PLATFORM_WINDOWS 
typedef  PAL::ReadOnlyWinGlobals PlatformGlobals;
#endif


BRWL_NS_END
