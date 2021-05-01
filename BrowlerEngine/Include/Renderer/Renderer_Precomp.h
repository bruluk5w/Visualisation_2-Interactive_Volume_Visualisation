#pragma once // (c) 2020 Lukas Brunner

#include "Common/Common_Precomp.h"


#define RENDERER Renderer

#define RENDERER_NS namespace RENDERER {

#define RENDERER_NS_END }

#define BRWL_RENDERER_NS BRWL_NS \
RENDERER_NS

#define BRWL_RENDERER_NS_END RENDERER_NS_END \
BRWL_NS_END
