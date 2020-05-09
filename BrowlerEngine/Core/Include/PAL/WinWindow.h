#pragma once

#include "Globals.h"

BRWL_PAL_NS

struct WinWindowImpl;

class WinWindow
{
	friend struct WinWindowImpl;
public:
	WinWindow(PlatformGlobals* globals);
	void create();
	void destroy();
	int width();
	int height();
	int x();
	int y();

	long handleMessage(unsigned int msg, unsigned int wParam, long lParam);

protected:

	PlatformGlobals* globals;
	std::unique_ptr<WinWindowImpl> impl;
	
};


BRWL_PAL_NS_END
