// Input class
// Stores current keyboard and mouse state include, pressed keys, mouse button pressed and mouse position.
// @author Paul Robertson

#include "Input.h"

void Input::SetKeyDown(unsigned char key)
{
	keys[key] = true;
}

void Input::SetKeyUp(unsigned char key)
{
	keys[key] = false;
}

bool Input::isKeyDown(int key)
{
	return keys[key];
}

void Input::SetSpecialKeyDown(unsigned char key)
{
	specialKeys[key] = true;
}

void Input::SetSpecialKeyUp(unsigned char key)
{
	specialKeys[key] = false;
}

bool Input::isSpecialKeyDown(int key)
{
	return specialKeys[key];
}

void Input::setMouseX(int pos)
{
	mouse.x = pos;
}

void Input::setMouseY(int pos)
{
	mouse.y = pos;
}

void Input::setMousePos(int ix, int iy)
{
	mouse.x = ix;
	mouse.y = iy;
}

int Input::getMouseX()
{
	return mouse.x;
}

int Input:: getMouseY()
{
	return mouse.y;
}

void Input::setLeftMouseButton(bool b)
{
	mouse.left = b;
}

bool Input::isLeftMouseButtonPressed()
{
	return mouse.left;
}

void Input::setRightMouseButton(bool b)
{
	mouse.right = b;
}

bool Input::isRightMouseButtonPressed()
{
	return mouse.right;
}

void Input::setMiddleMouseButton(bool b)
{
	mouse.middle = b;
}

bool Input::isMiddleMouseButtonPressed()
{
	return mouse.middle;
}

void Input::setScrollUpMouseWheel(bool b)
{
	mouse.scroll_up = b;
}

bool Input::isScrollUpMouseWheel()
{
	return mouse.scroll_up;
}

void Input::setScrollDownMouseWheel(bool b)
{
	mouse.scroll_down = b;
}

bool Input::isScrollDownMouseWheel()
{
	return mouse.scroll_down;
}

