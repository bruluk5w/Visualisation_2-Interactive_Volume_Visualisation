#pragma once

#ifdef BRWL_PLATFORM_WINDOWS

#include "Globals.h"
#include "Common/EventSystem.h"
#include "Events.h"

BRWL_PAL_NS

struct WinWindowImpl;

class WinWindow
{
	friend struct WinWindowImpl;
public:
	WinWindow(PlatformGlobals* globals, EventBusSwitch<Event>* eventSystem);
	virtual ~WinWindow();
	// A window must only be created from the same thread that will also call "processPlatformMessages"
	void create(int x, int y, int width, int height);
	void destroy();
	int x() const;
	int y() const;
	int width() const;
	int height() const;
	// only call this method if the window has been created in it's own thread and 
	// does not receive message updates from the parent window
	void processPlatformMessages();

protected:
	void move(int x, int y, int dx, int dy);
	void resize(int width, int height);

	PlatformGlobals* globals;
	std::unique_ptr<WinWindowImpl> impl;

	EventBusSwitch<Event>* eventSystem;
};


BRWL_PAL_NS_END

#endif // BRWL_PLATFORM_WINDOWS