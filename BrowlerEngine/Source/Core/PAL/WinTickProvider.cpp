#include "Timer.h"

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_NS

//#pragma optimize("", off)
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

uint64_t TickProvider::forceGetTick()
{
	LARGE_INTEGER x;
	QueryPerformanceCounter(&x);
	return x.QuadPart;
}


BRWL_NS_END

#endif // BRWL_PLATFORM_WINDOWS
