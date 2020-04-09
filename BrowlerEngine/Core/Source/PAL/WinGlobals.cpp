#include "PAL/WinGlobals.h"

#include "PAL/BrowlerWindowsInclude.h"

BRWL_PAL_NS


WinGlobals::WinGlobals(const HINSTANCE& hInstance, const LPWSTR cmdLine, const int& cmdShow) :
	hInstance(hInstance),
	cmdLine(cmdLine),
	cmdShow(cmdShow)
{ }


ReadOnlyWinGlobals::ReadOnlyWinGlobals(const WinGlobals& globals) :
	globals(globals)
{ }

const HINSTANCE& ReadOnlyWinGlobals::GetHInstance() const { return globals.hInstance; }

const LPWSTR& ReadOnlyWinGlobals::GetCmdLine() const { return globals.cmdLine; }

const int& ReadOnlyWinGlobals::GetCmdShow() const { return globals.cmdShow; }


BRWL_PAL_NS_END
