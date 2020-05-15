#pragma once

#include "Globals.h"
#include "Common/EventSystem.h"
#include "Events.h"

BRWL_PAL_NS

struct WinWindowImpl;

class WinWindow
{
	friend struct WinWindowImpl;
public:
	WinWindow(PlatformGlobals* globals, EventSystem<Event>* eventSystem);
	void create(int x, int y, int width, int height);
	void destroy();
	int x() const;
	int y() const;
	int width() const;
	int height() const;

protected:
	void move(int x, int y, int dx, int dy);
	void resize(int width, int height);

	PlatformGlobals* globals;
	std::unique_ptr<WinWindowImpl> impl;

	EventSystem<Event>* eventSystem;
};


BRWL_PAL_NS_END
