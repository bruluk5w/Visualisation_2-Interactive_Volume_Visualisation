#pragma once

#include "Common/Events.h"

BRWL_NS


enum class Event {
	WINDOW_RESIZE,
	WINDOW_MOVE,
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

EVENT_PARAMETER_MAP(Event, WINDOW_RESIZE, WindowSizeParam, WINDOW_MOVE, WindowMoveParam)



//
//struct MouseMoveParam
//{
//	double x;
//	double y;
//};
//
//template<>
//struct TypeConvert<Event::MOUSE_MOVE> {
//	using ResultType = MouseMoveParam;
//};
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

// Add more parameter structs here and evtl. also add specializations of the TypeConvert helper struct


BRWL_NS_END