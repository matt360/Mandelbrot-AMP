// Data and Structures 2 (CMP202) - Assessment Project
// @author Matthew Wallace <1502616@abertay.ac.uk>
#include <freeglut.h>
#include "dependencies.h"
#include "mandelbrot.h"
#include "input.h"
#include "Camera.h"
#include "FreeCamera.h"

const unsigned WINDOW_WIDTH = 1024;
const unsigned WINDOW_HEIGHT = 1024;
const unsigned WINDOW_INIT_X = 450;
const unsigned WINDOW_INIT_Y = 100;

int oldTimeSinceStart = 0;

//Input * input = new Input();
//Mandelbrot * mandelbrot = new Mandelbrot(input);
Input * input;
Mandelbrot * mandelbrot;

// Handles keyboard input events from GLUT.
// Called whenever a "normal" key is pressed.
// Normal keys are defined as any key not including the F keys, CTRL, SHIFT, ALT, etc.
// Key press is recorded in Input class
// Parameters include key pressed and current mouse x, y coordinates.
// Mouse coordinates are handled separately.
void processNormalKeys(unsigned char key, int x, int y)
{
	// If the ESCAPE key was pressed, exit application.
	if (key == 27)	// Escape key (in non-windows you can use 27, the ASCII value for escape)
		exit(0);
	// Send key down to input class.
	input->SetKeyDown(key);
}

// Handles keyboard input events from GLUT.
// Called whenever a "normal" key is released.
// Normal keys are defined as any key not including the F keys, CTRL, SHIFT, ALT, arrow keys, etc.
// Key press is recorded in Input class
// Parameters include key pressed and current mouse x, y coordinates.
// Mouse coordinates are handled separately.
void processNormalKeysUp(unsigned char key, int x, int y)
{
	// Send key up to input class.
	input->SetKeyUp(key);
}

// Handles keyboard input events from GLUT.
// Called whenever a "special" key is pressed.
// Special keys are defined as F keys, CTRL, SHIFT, ALT, arrow keys, etc
// Currently a place holder function, can be utilised if required.
// Parameters include key pressed and current mouse x, y coordinates.
// Mouse coordinates are handled separately.
void processSpecialKeys(int key, int x, int y)
{
	input->SetSpecialKeyDown(key);
}

// Handles keyboard input events from GLUT.
// Called whenever a "special" key is released.
// Special keys are defined as F keys, CTRL, SHIFT, ALT, arrow keys, etc
// Currently a place holder function, can be utilised if required.
// Parameters include key pressed and current mouse x, y coordinates.
// Mouse coordinates are handled separately.
void processSpecialKeysUp(int key, int x, int y)
{
	input->SetSpecialKeyUp(key);
}

// Handles mouse movement events from GLUT.
// Active movement is define as mouse movement while a mouse button is pressed.
// Called every loop. Parameters are the new x, y coordinates of the mouse.
void processActiveMouseMove(int x, int y)
{
	// Record mouse position in Input class.
	input->setMousePos(x, y);
}

// Handles mouse movement events from GLUT.
// Passive mouse movement is define as mouse movement without a mouse button is pressed.
// Called every loop. Parameters are the new x, y coordinates of the mouse.
void processPassiveMouseMove(int x, int y)
{
	// Record mouse position in Input class.
	input->setMousePos(x, y);
}

// Handles mouse button events from GLUT.
// Parameters include mouse button that fired the event (GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON),
// button state (up and down), and current cursor position.
void processMouseButtons(int button, int state, int x, int y)
{
	enum
	{
		MOUSE_LEFT_BUTTON = 0,
		MOUSE_MIDDLE_BUTTON = 1,
		MOUSE_RIGHT_BUTTON = 2,
		MOUSE_SCROLL_UP = 3,
		MOUSE_SCROLL_DOWN = 4,
		MOUSE_BUTTON_DOWN = 0,
		MOUSE_BUTTON_UP = 1
	};

	switch (button) {
		// Detect left button press/released
	case MOUSE_LEFT_BUTTON: 
	{
		switch (state) {
		case MOUSE_BUTTON_DOWN: input->setLeftMouseButton(true);
			break;
		default: input->setLeftMouseButton(false);
			break;
		}
	} break;
		// Detect middle button press/released
	case MOUSE_MIDDLE_BUTTON: 
	{
		switch (state) {
		case MOUSE_BUTTON_DOWN: input->setMiddleMouseButton(true);
			break;
		default: input->setMiddleMouseButton(false);
			break;
		}
	} break;
		// Detect right button press/released
	case MOUSE_RIGHT_BUTTON: 
	{
		switch (state) {
		case MOUSE_BUTTON_DOWN: input->setRightMouseButton(true);
			break;
		default: input->setRightMouseButton(false);
			break;
		}
	} break;
	// Detect mouse wheel scroll up (setScrollUpMouseWheel(false) must be called in update() after isScrollUpMouseWheel())  
	case MOUSE_SCROLL_UP: 
	{
		switch (state) {
		case MOUSE_BUTTON_DOWN: input->setScrollUpMouseWheel(true);
			break;
		}
	} break;
	// Detect mouse wheel scroll down (setScrollDownMouseWheel(false) must be called in update() after isScrollDownMouseWheel())
	case MOUSE_SCROLL_DOWN: 
	{
		switch (state) {
		case MOUSE_BUTTON_DOWN: input->setScrollDownMouseWheel(true);
			break;
		}
	} break;
	}
}

void changeSize(int w, int h)
{
	/*int width = w;
	int height = h;*/
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = (float)w / (float)h;
	float fov = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(fov, ratio, nearPlane, farPlane);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void renderScene()
{
	// Calculate delta time.
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (float)timeSinceStart - (float)oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	deltaTime = deltaTime / 100.0f;

	mandelbrot->update(deltaTime);
	mandelbrot->render();

	// Swap buffers, after all objects are rendered.
	glutSwapBuffers();
}

int main(int argc, char *argv[])
{
	// Init GLUT and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(WINDOW_INIT_X, DM_YRESOLUTION / 1000);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Mandelbrot");

	// Register callback functions for change in size and rendering.
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	// Register Input callback functions.
	// 'Normal' keys processing
	glutKeyboardFunc(processNormalKeys);
	glutKeyboardUpFunc(processNormalKeysUp);
	// Special keys processing
	glutSpecialFunc(processSpecialKeys);
	glutSpecialUpFunc(processSpecialKeysUp);

	// Mouse callbacks
	glutMotionFunc(processActiveMouseMove);
	glutPassiveMotionFunc(processPassiveMouseMove);
	// void glutMouseFunc(void(*func)(int button, int state, int x, int y))
	glutMouseFunc(processMouseButtons);

	input = new Input();
	mandelbrot = new Mandelbrot(input);

	// Enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}
