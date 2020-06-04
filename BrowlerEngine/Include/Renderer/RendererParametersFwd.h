#pragma once // (c) 2020 Lukas Brunner

BRWL_RENDERER_NS


namespace PAL
{
#ifdef BRWL_PLATFORM_WINDOWS
	struct WinRendererParameters;
#endif
}

#ifdef BRWL_PLATFORM_WINDOWS
typedef PAL::WinRendererParameters RendererParameters;
#endif


BRWL_RENDERER_NS_END