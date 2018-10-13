#pragma once

#include <GL\glfw3.h>
#include "glm\glm.hpp"

using namespace glm;

class InputHandler {
public:
	void init(GLFWwindow * window);

	bool isKeyPressed(int key);
	bool isMouseButtonPressed(int button);
	vec2 getMousePos();
	vec2 getWindowSize();

private:
	GLFWwindow * window;
};