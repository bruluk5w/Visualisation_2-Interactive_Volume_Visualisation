#pragma once // (c) 2020 Lukas Brunner

#include "Common/Events.h"

BRWL_RENDERER_NS


enum class Event {
	WINDOW_RESIZE,
	WINDOW_MOVE,
	FRAMEBUFFER_RESIZE,
	FRAME_IDX_CHANGE,
	CAM_HAS_MOVED,
	MAX,
	MIN = 0
};


struct FrameBufferResizeParam
{
	int width;
	int height;
};

struct WindowSizeParam
{
	int width;
	int height;
};

struct WindowMoveParam
{
	int x;  // the new x position
	int y;  // the new y position
	int dx; // the change in the x coordinate
	int dy; // the change in the y coordinate
};

struct FrameIdxChange
{
	size_t newFrameIdx;
};

BRWL_RENDERER_NS_END


BRWL_NS



EVENT_PARAMETER_MAP_START
	MAP_EVENT_PARAMETER(RENDERER::Event, WINDOW_MOVE, RENDERER::WindowMoveParam)
	MAP_EVENT_PARAMETER(RENDERER::Event, WINDOW_RESIZE, RENDERER::WindowSizeParam)
	MAP_EVENT_PARAMETER(RENDERER::Event, FRAMEBUFFER_RESIZE, RENDERER::FrameBufferResizeParam)
	MAP_EVENT_PARAMETER(RENDERER::Event, FRAME_IDX_CHANGE, RENDERER::FrameIdxChange)
EVENT_PARAMETER_MAP_END

BRWL_NS_END