#pragma once

BRWL_RENDERER_NS


namespace PAL
{

	class WinRenderer;
#ifdef BRWL_PLATFORM_WINDOWS
	typedef WinRenderer  PlatformRenderer;
#endif

}


class Renderer;


BRWL_RENDERER_NS_END