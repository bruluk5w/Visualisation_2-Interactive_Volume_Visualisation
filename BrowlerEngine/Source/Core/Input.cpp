#include "Input.h"

BRWL_NS


InputManager::InputManager() :
	keyPressed(),
	buttonPressed(),
	mouseX(0),
	mouseY(0),
	lastMouseX(0),
	lastMouseY(0),
	mouseDeltaX(0),
	mouseDeltaY(0)
{ }

InputManager::~InputManager()
{ }


bool InputManager::isKeyPressed(Key k) const
{
	return keyPressed[ENUM_CLASS_TO_NUM(k)];
}

bool InputManager::isKeyDown(Key btn) const
{
	return keyPressed[ENUM_CLASS_TO_NUM(btn)] && !wasKeyPressed[ENUM_CLASS_TO_NUM(btn)];
}

bool InputManager::isKeyUp(Key btn) const
{
	return !keyPressed[ENUM_CLASS_TO_NUM(btn)] && wasKeyPressed[ENUM_CLASS_TO_NUM(btn)];
}

bool InputManager::isButtonPressed(Button btn) const
{
	return buttonPressed[ENUM_CLASS_TO_NUM(btn)];
}

bool InputManager::isButtonDown(Button btn) const
{
	return buttonPressed[ENUM_CLASS_TO_NUM(btn)] && !wasButtonPressed[ENUM_CLASS_TO_NUM(btn)];
}

bool InputManager::isButtonUp(Button btn) const
{
	return !buttonPressed[ENUM_CLASS_TO_NUM(btn)] && wasButtonPressed[ENUM_CLASS_TO_NUM(btn)];
}

void InputManager::preMessageUpdate()
{
	wasKeyPressed = keyPressed;
	wasButtonPressed = buttonPressed;
}

void InputManager::postMessageUpdate()
{
	mouseDeltaX = mouseX - lastMouseX;
	mouseDeltaY = mouseY - lastMouseY;
	lastMouseX = mouseX;
	lastMouseY = mouseY;

	scrollX = nextScrollX;
	scrollY = nextScrollY;
	nextScrollX = nextScrollY = 0.0;

}

void InputManager::unfocus()
{
	buttonPressed.reset();
	keyPressed.reset();
}

void InputManager::focus()
{ }

void InputManager::consumeKeyEvent(uint64_t key, bool isDown)
{
	// Keys 0 - 9
	if (0x30 <= key && key <= 0x39) {
		keyPressed[ENUM_CLASS_TO_NUM(Key::NUM0) + (key - 0x30)] = isDown;
		return;
	}
	// Keys A - Z
	if (0x41 <= key && key <= 0x5A) {
		keyPressed[ENUM_CLASS_TO_NUM(Key::A) + (key - 0x41)] = isDown;
		return;
	}
	//
	//action is either GLFW_PRESS or GLFW_RELEASE 
	switch (key)
	{
	case(VK_LBUTTON):
		buttonPressed[ENUM_CLASS_TO_NUM(Button::MOUSE_1)] = isDown;
		break;
	case(VK_RBUTTON):
		buttonPressed[ENUM_CLASS_TO_NUM(Button::MOUSE_2)] = isDown;
		break;
	case(VK_MBUTTON):
		buttonPressed[ENUM_CLASS_TO_NUM(Button::MOUSE_3)] = isDown;
		break;
	case(VK_XBUTTON1):
		buttonPressed[ENUM_CLASS_TO_NUM(Button::MOUSE_4)] = isDown;
		break;
	case(VK_XBUTTON2):
		buttonPressed[ENUM_CLASS_TO_NUM(Button::MOUSE_5)] = isDown;
		break;
	case(VK_ESCAPE):
		keyPressed[ENUM_CLASS_TO_NUM(Key::ESC)] = isDown;
		break;
	case(VK_RETURN):
		keyPressed[ENUM_CLASS_TO_NUM(Key::ENTER)] = isDown;
		break;
	case(VK_BACK):
		keyPressed[ENUM_CLASS_TO_NUM(Key::BACKSPACE)] = isDown;
		break;
	case(VK_TAB):
		keyPressed[ENUM_CLASS_TO_NUM(Key::TAB)] = isDown;
		break;
	case(VK_SHIFT):
		keyPressed[ENUM_CLASS_TO_NUM(Key::SHIFT)] = isDown;
		break;
	case(VK_CONTROL):
		keyPressed[ENUM_CLASS_TO_NUM(Key::CTRL)] = isDown;
		break;
	case(VK_MENU):
		keyPressed[ENUM_CLASS_TO_NUM(Key::ALT)] = isDown;
		break;
	case(VK_SPACE):
		keyPressed[ENUM_CLASS_TO_NUM(Key::SPACE)] = isDown;
		break;
	case(VK_UP):
		keyPressed[ENUM_CLASS_TO_NUM(Key::UP)] = isDown;
		break;
	case(VK_DOWN):
		keyPressed[ENUM_CLASS_TO_NUM(Key::DOWN)] = isDown;
		break;
	case(VK_LEFT):
		keyPressed[ENUM_CLASS_TO_NUM(Key::LEFT)] = isDown;
		break;
	case(VK_RIGHT):
		keyPressed[ENUM_CLASS_TO_NUM(Key::RIGHT)] = isDown;
		break;
	case(VK_F1):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F1)] = isDown;
		break;
	case(VK_F2):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F2)] = isDown;
		break;
	case(VK_F3):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F3)] = isDown;
		break;
	case(VK_F4):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F4)] = isDown;
		break;
	case(VK_F5):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F5)] = isDown;
		break;
	case(VK_F6):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F6)] = isDown;
		break;
	case(VK_F7):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F7)] = isDown;
		break;
	case(VK_F8):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F8)] = isDown;
		break;
	case(VK_F9):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F9)] = isDown;
		break;
	case(VK_F10):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F10)] = isDown;
		break;
	case(VK_F11):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F11)] = isDown;
		break;
	case(VK_F12):
		keyPressed[ENUM_CLASS_TO_NUM(Key::F12)] = isDown;
		break;
	default:
		break;
	}
}

void InputManager::consumeMousePositionEvent(double x, double y)
{
	mouseX = x;
	mouseY = y;
}

void InputManager::consumeScrollEvent(double dx, double dy)
{
	nextScrollX += dx;
	nextScrollY += dy;
}

BRWL_NS_END