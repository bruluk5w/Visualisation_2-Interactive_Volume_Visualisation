#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

#include "resource.h"

BRWL_NS

namespace PAL
{
	class WinGlobalsAccessor;

	class WinGlobals final
	{
	public:
		friend class WinGlobalsAccessor;

		WinGlobals(const HINSTANCE& hInstance, const LPWSTR cmdLine, const int& cmdShow) :
			hInstance(hInstance),
			cmdLine(cmdLine),
			cmdShow(cmdShow)
		{ }
	private:
		const HINSTANCE	hInstance;
		const LPWSTR	cmdLine;
		const int		cmdShow;
	};

	class WinGlobalsAccessor final
	{
	public:
		WinGlobalsAccessor(const WinGlobals& globals) : globals(globals)
		{ }

		const HINSTANCE& GetHInstance() const { return globals.hInstance; }
		const LPWSTR& GetCmdLine() const { return globals.cmdLine; }
		const int& GetCmdShow() const { return globals.cmdShow; }

	private:
		const WinGlobals& globals;
	};
}

BRWL_NS_END

#endif // BRWL_PLATFORM_WINDOWS
