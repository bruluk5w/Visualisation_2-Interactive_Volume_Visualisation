#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS


namespace PAL
{


	struct WinRendererParameters
	{
		HWND hWnd;
		struct Dimensions {
			int width, height;
		} initialDimensions;
	};


}

BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS
