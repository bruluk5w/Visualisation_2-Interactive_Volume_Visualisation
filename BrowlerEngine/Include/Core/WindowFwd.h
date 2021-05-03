#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_PAL_NS
class WinWindow;
BRWL_PAL_NS_END

BRWL_NS
typedef  PAL::WinWindow Window;
BRWL_NS_END

#endif
