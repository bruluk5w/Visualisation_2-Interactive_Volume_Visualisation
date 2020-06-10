#pragma once // (c) 2020 Lukas Brunner

#include <bitset>

BRWL_NS


enum class Key {
	ESC = 0,
	ENTER,
	BACKSPACE,
	TAB,
	SHIFT,
	CTRL,
	SPACE,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
	A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
	MAX,
	MIN = 0
};

enum class Button {
	MOUSE_1 = 0,
	MOUSE_2,
	MOUSE_3,
	MOUSE_4,
	MOUSE_5,

	MAX,
	MIN = 0
};

#ifdef BRWL_PLATFORM_WINDOWS
namespace PAL {
	struct WinWindowImpl;
}
#endif

class InputManager
{
#ifdef BRWL_PLATFORM_WINDOWS
	friend struct PAL::WinWindowImpl;
#endif
	friend class Engine;
public:
	InputManager();

	~InputManager();

	bool isKeyPressed(Key k) const;
	bool isMouseButtonPressed(Button btn) const;
	double getMouseX() const { return mouseX; }
	double getMouseY() const { return mouseY; }
	double getMouseDeltaX() const { return mouseDeltaX; }
	double getMouseDeltaY() const { return mouseDeltaY; }
	double getScrollDeltaX() const { return scrollX; }
	double getScrollDeltaY() const { return scrollY; }
protected:
	void update();
	void consumeKeyEvent(uint64_t key, bool isDown);
	void consumeMousePositionEvent(double x, double y);
	void consumeScrollEvent(double dx, double dy);


private:
	std::bitset<ENUM_CLASS_TO_NUM(Key::MAX)> keyPressed;
	std::bitset<ENUM_CLASS_TO_NUM(Button::MAX)> buttonPressed;
	double mouseX;
	double mouseY;
	double mouseDeltaX;
	double mouseDeltaY;

	double lastMouseX;
	double lastMouseY;

	double scrollX;
	double nextScrollX;
	double scrollY;
	double nextScrollY;
};


BRWL_NS_END