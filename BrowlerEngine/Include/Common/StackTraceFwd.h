#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_PAL_NS
class WinStackTrace;
BRWL_PAL_NS_END

BRWL_NS
typedef  PAL::WinStackTrace StackTrace;
BRWL_NS_END

#endif
