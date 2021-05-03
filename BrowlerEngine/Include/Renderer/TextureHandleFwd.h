#pragma once

BRWL_RENDERER_NS


namespace PAL
{
#ifdef BRWL_PLATFORM_WINDOWS
	struct WinTextureHandle;
#endif
}

#ifdef BRWL_PLATFORM_WINDOWS
typedef PAL::WinTextureHandle TextureHandle;
#endif


BRWL_RENDERER_NS_END