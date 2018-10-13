#include "InputHandler.h"

void InputHandler::init(GLFWwindow * _window)
{
	window = _window;
}

bool InputHandler::isKeyPressed(int key)
{
	return glfwGetKey(window, key) == GLFW_PRESS;
}

bool InputHandler::isMouseButtonPressed(int button)
{
	return glfwGetMouseButton(window, button);
}

vec2 InputHandler::getMousePos()
{
	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	return vec2(mx, my);
}

vec2 InputHandler::getWindowSize()
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	return vec2(w, h);
}