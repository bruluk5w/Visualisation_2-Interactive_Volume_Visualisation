#include "Timer.h"

#include "Common/PAL/BrowlerWindowsInclude.h"

BRWL_NS

TickProvider::TickProvider() :
	tick(0),
	frequency(0),
	timer(nullptr)
{
	LARGE_INTEGER x;
	QueryPerformanceFrequency(&x);
	frequency = x.QuadPart;

	nextFrame(); // initialize tick count to a current sane number
}

void TickProvider::nextFrame()
{
	LARGE_INTEGER x;
	QueryPerformanceCounter(&x);
	tick = x.QuadPart;
	if (timer != nullptr)
	{
		timer->onTickUpdated();
	}
}

BRWL_NS_END