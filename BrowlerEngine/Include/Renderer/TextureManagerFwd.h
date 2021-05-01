#pragma once

BRWL_RENDERER_NS


namespace PAL
{
#ifdef BRWL_PLATFORM_WINDOWS
	class WinTextureManager;
#endif
}

#ifdef BRWL_PLATFORM_WINDOWS
typedef PAL::WinTextureManager TextureManager;
#endif


BRWL_RENDERER_NS_END