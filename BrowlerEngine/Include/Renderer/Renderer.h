#pragma once

#include "PAL/WinRenderer.h"

BRWL_NS

namespace Rendering
{

#ifdef BRWL_PLATFORM_WINDOWS
	typedef PAL::WinRenderer  PlatformRenderer;
#endif

}

BRWL_NS_END