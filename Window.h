#pragma once

#include <GL\glew.h>
#include <GL\glfw3.h>
#include "InputHandler.h"

class Window {
public:
	InputHandler input;

	Window(const char * _title, unsigned int _width, unsigned int _height, bool _fullscreen);
	~Window();

	bool			closed();
	void			update();
	void			waitUpdate();
	GLFWwindow *	get();

private:
	GLFWwindow *	window;
	const char *	title;
	bool			fullscreen;

	static void saveScreenshot(GLFWwindow* window);
	static void resize_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};