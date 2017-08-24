// Input class
// Stores current keyboard and mouse state include, pressed keys, mouse button pressed and mouse position.
// @author Paul Robertson
// mouse and special keys functions extended 
// @author Matthew Wallace
#pragma once

class Input
{
	// Mouse struct stores mouse related data include cursor
	// x, y coordinates and left/right button pressed state.
	struct Mouse
	{
		int x,y;
		bool left, middle, right, scroll_up, scroll_down;
	};

public:
	// Getters and setters for keys
	void SetKeyDown(unsigned char key);
	void SetKeyUp(unsigned char key);
	bool isKeyDown(int);

	void SetSpecialKeyDown(unsigned char key);
	void SetSpecialKeyUp(unsigned char key);
	bool isSpecialKeyDown(int key);

	// getters and setters for mouse buttons and position.
	void setMouseX(int);
	void setMouseY(int);
	void setMousePos(int x, int y);
	int getMouseX();
	int getMouseY();
	void setLeftMouseButton(bool b);
	bool isLeftMouseButtonPressed();
	void setRightMouseButton(bool b);
	bool isRightMouseButtonPressed();
	void setMiddleMouseButton(bool b);
	bool isMiddleMouseButtonPressed();
	void setScrollUpMouseWheel(bool b);
	bool isScrollUpMouseWheel();
	void setScrollDownMouseWheel(bool b);
	bool isScrollDownMouseWheel();

private:
	// Boolean array, element per key
	// Mouse struct object.
	bool keys[256];
	bool specialKeys[256];
	Mouse mouse;

};