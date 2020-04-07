#pragma once

#include "PAL/WinThread.h"
#include "ThreadState.h"

#include <thread>

BRWL_PAL_NS


// putting this into PAL namespace beause Thread should be used insteaad of PlatformThread
#ifdef BRWL_PLATFORM_WINDOWS
template<class returnType>
using PlatformThread = WinThread<returnType>;
#endif


BRWL_PAL_NS_END

BRWL_NS

template<class returnType>
class Thread : public PAL::PlatformThread<returnType>
{
	using PlatformImpl = PAL::PlatformThread<returnType>;

public:
	static int hardwareConcurrency = [] { return std::thread::hardware_concurrency(); }();

	Thread(PlatformImpl::ThreadTarget target, PlatformImpl::ReturnType* returnValue) :
		PlatformImpl(std::move(target), returnValue)
	{ }
};


BRWL_NS_END