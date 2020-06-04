#pragma once // (c) 2020 Lukas Brunner

BRWL_RENDERER_NS


namespace PAL
{
#ifdef BRWL_PLATFORM_WINDOWS
	class WinRenderer;
#endif
}

#ifdef BRWL_PLATFORM_WINDOWS
typedef PAL::WinRenderer Renderer;
#endif


BRWL_RENDERER_NS_END