#pragma once

#include "Common/Events.h"

BRWL_RENDERER_NS


enum class Event {
	FRAMEBUFFER_RESIZE,
	MAX,
	MIN = 0
};

BRWL_RENDERER_NS_END


BRWL_NS


struct FrameBufferResizeParam
{
	int width;
	int height;
};

//EVENT_PARAMETER_MAP(RENDERER::Event, FRAMEBUFFER_RESIZE, FrameBufferResizeParam)


BRWL_NS_END