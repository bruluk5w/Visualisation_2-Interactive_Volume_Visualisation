#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS


namespace PAL
{

	struct WinRendererParameters
	{
		HWND hWnd;
	};

}

BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS