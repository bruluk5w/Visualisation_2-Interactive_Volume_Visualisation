#pragma once // (c) 2020 Lukas Brunner

#include "Common/Events.h"

BRWL_NS


enum class Event {
	WINDOW_RESIZE,
	WINDOW_MOVE,
	FRAME_IDX_CHANGE,
	SET_FREE_CAM_MOVEMENT,
	//KEY_F1,
	//KEY_F2,
	//KEY_F3,
	//KEY_F4,
	//KEY_F5,
	//KEY_F6,
	//KEY_F7,
	//KEY_F8,
	//KEY_F9,
	//KEY_F10,
	//KEY_F11,
	//KEY_F12,
	//KEY_W,
	//KEY_S,
	//KEY_A,
	//KEY_D,
	//KEY_Q,
	//KEY_E,
	//KEY_ESC,
	//KEY_MOUSE_LEFT,
	//KEY_MOUSE_RIGHT,
	//KEY_MOUSE_MIDDLE,
	//MOUSE_MOVE,
	//MOUSE_DELTA,
	//MOUSE_SCROLL,
	MAX,
	MIN = 0
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

struct BoolParam
{
	bool value;
};

EVENT_PARAMETER_MAP_START
	MAP_EVENT_PARAMETER(Event, WINDOW_MOVE, WindowMoveParam)
	MAP_EVENT_PARAMETER(Event, WINDOW_RESIZE, WindowSizeParam)
	MAP_EVENT_PARAMETER(Event, FRAME_IDX_CHANGE, FrameIdxChange)
	MAP_EVENT_PARAMETER(Event, SET_FREE_CAM_MOVEMENT, BoolParam)
EVENT_PARAMETER_MAP_END


//
//struct MouseMoveParam
//{
//	double x;
//	double y;
//};
//
//
//struct MouseDeltaParam
//{
//	double dx;
//	double dy;
//};
//
//template<>
//struct TypeConvert<Event::MOUSE_DELTA> {
//	using ResultType = MouseDeltaParam;
//};
//
//template<>
//struct TypeConvert<Event::MOUSE_SCROLL> {
//	using ResultType = MouseDeltaParam;
//};

BRWL_NS_END