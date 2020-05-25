#include "PAL/WinGlobals.h"

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_PAL_NS


WinGlobals::WinGlobals(const HINSTANCE& hInstance, const BRWL_CHAR* cmdLine, const int& cmdShow) :
	hInstance(hInstance),
	cmdLine(cmdLine),
	cmdShow(cmdShow)
{ }


ReadOnlyWinGlobals::ReadOnlyWinGlobals(const WinGlobals& globals) :
	globals(globals)
{ }

const HINSTANCE& ReadOnlyWinGlobals::GetHInstance() const { return globals.hInstance; }

const BRWL_CHAR* ReadOnlyWinGlobals::GetCmdLine() const { return globals.cmdLine; }

const int& ReadOnlyWinGlobals::GetCmdShow() const { return globals.cmdShow; }


BRWL_PAL_NS_END

#endif // BRWL_PLATFORM_WINDOWS
