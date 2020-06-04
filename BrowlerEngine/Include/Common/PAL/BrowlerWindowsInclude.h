#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
// undefine annoying macros from windows headers
#undef near
#undef far
#ifdef ERROR
#undef ERROR
#endif

#endif