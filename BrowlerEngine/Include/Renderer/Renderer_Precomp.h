// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// reference additional headers your program requires here
#include "Common/Common_Precomp.h"

#define RENDERER Renderer

#define RENDERER_NS namespace RENDERER {

#define RENDERER_NS_END }

#define BRWL_RENDERER_NS BRWL_NS \
RENDERER_NS

#define BRWL_RENDERER_NS_END RENDERER_NS_END \
BRWL_NS_END