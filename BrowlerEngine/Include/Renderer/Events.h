#pragma once

#include "Common/Events.h"

BRWL_RENDERER_NS


enum class Event {
	WINDOW_RESIZE,
	FRAMEBUFFER_RESIZE,
	MAX,
	MIN = 0
};

BRWL_RENDERER_NS_END


BRWL_NS


template<RENDERER::Event event>
typename TypeConvert<RENDERER::Event, event>::ResultType* castParam(void* param) {
	return static_cast<typename TypeConvert<event>::ResultType*>(param);
}



struct WindowResizeParam
{
	int width;
	int height;
};

template<>
struct TypeConvert<RENDERER::Event::WINDOW_RESIZE> {
	using ResultType = WindowResizeParam;
};

struct FrameBufferResizeParam
{
	int width;
	int height;
};

template<>
struct TypeConvert<RENDERER::Event::FRAMEBUFFER_RESIZE> {
	using ResultType = FrameBufferResizeParam;
};

BRWL_NS_END