#pragma once

BRWL_NS


enum class Event {
	//WINDOW_RESIZE,
	//FRAMEBUFFER_RESIZE,
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

template<Event event>
struct TypeConvert {
	static_assert(std::integral_constant<bool, false>, "No type convert defined for this type.");
};

template<Event event>
typename TypeConvert<event>::ResultType* castParam(void* param) {
	return static_cast<typename TypeConvert<event>::ResultType*>(param);
}

struct WindowResizeParam
{
	int width;
	int height;
};

template<>
struct TypeConvert<Event::WINDOW_RESIZE> {
	using ResultType = WindowResizeParam;
};

struct FrameBufferResizeParam
{
	int width;
	int height;
};

template<>
struct TypeConvert<Event::FRAMEBUFFER_RESIZE> {
	using ResultType = FrameBufferResizeParam;
};

struct MouseMoveParam
{
	double x;
	double y;
};

template<>
struct TypeConvert<Event::MOUSE_MOVE> {
	using ResultType = MouseMoveParam;
};

struct MouseDeltaParam
{
	double dx;
	double dy;
};

template<>
struct TypeConvert<Event::MOUSE_DELTA> {
	using ResultType = MouseDeltaParam;
};

template<>
struct TypeConvert<Event::MOUSE_SCROLL> {
	using ResultType = MouseDeltaParam;
};

// Add more parameter structs here and evtl. also add specializations of the TypeConvert helper struct


BRWL_NS_END