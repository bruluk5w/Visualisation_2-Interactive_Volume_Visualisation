// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#if defined _WIN32

#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
#define NOMINMAX			// Do not define evil min/max macros in windows headers
// Windows Header Files
#include <windows.h>
#undef NOMINMAX

#endif

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>


// reference additional headers your program requires here
#include "Core_Precomp.h"