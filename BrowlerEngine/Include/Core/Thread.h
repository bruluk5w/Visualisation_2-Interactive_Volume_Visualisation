#pragma once // (c) 2020 Lukas Brunner

#include "PAL/WinThread.h"
#include "ThreadState.h"

#include <thread>

BRWL_PAL_NS


// putting this into PAL namespace beause Thread should be used instead of PlatformThread
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
	static inline unsigned int hardwareConcurrency = std::thread::hardware_concurrency();
public:

	Thread(PlatformImpl::ThreadTarget target, PlatformImpl::ReturnType* returnValue) :
		PlatformImpl(std::move(target), returnValue)
	{ }

	static unsigned int getHardwareConcurrency() { return hardwareConcurrency; }
};


BRWL_NS_END