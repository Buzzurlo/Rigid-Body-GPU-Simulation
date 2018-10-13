#include "Window.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Window::Window(const char * _title, unsigned int _width, unsigned int _height, bool _fullscreen)
	: window(NULL), title(_title), fullscreen(_fullscreen)
{
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
	glfwWindowHint(GLFW_SAMPLES, 4);

																   // Open a window and create its OpenGL context
	window = glfwCreateWindow(_width, _height, title, fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. Try the 2.1 version of openGL.\n");
		exit(-2);
	}

	glfwMakeContextCurrent(window);
	glewExperimental = true; // needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		exit(-3);
	}

	glfwSetCursorPos(window, 0.0, 0.0);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	input.init(window);

	glfwSetWindowSizeCallback(window, resize_callback);
	glfwSetKeyCallback(window, key_callback);
}

Window::~Window()
{
	glfwTerminate();
}

void Window::update()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Window::waitUpdate()
{
	glfwSwapBuffers(window);
	glfwWaitEvents();
}

GLFWwindow * Window::get()
{
	return window;
}

bool Window::closed()
{
	return glfwWindowShouldClose(window);
}

void Window::saveScreenshot(GLFWwindow* window)
{
	FILE * bmp;
	int buffer;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	int i = 1;
	char name[100] = "screenshot1.bmp";
	char number[20];
	while ((bmp = fopen(name, "r")) != NULL) {
		strcpy(name, "screenshot");
		_itoa(++i, number, 10);
		strcat(name, number);
		strcat(name, ".bmp");
		fclose(bmp);
	}

	bmp = fopen(name, "wb+");

	// HEADER
	buffer = 0x4D42;
	fwrite(&buffer, sizeof(char), 2, bmp);   // Signature
	buffer = width * height * 3 + 54;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Size of bmp (in bytes)
	buffer = 0;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Reserved 1 and 2
	buffer = 54;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Offset to image data
	buffer = 40;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Size of BITMAPINFOHEADER = 40
	buffer = width;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Image width (pixels)
	buffer = height;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Image height
	buffer = 1;
	fwrite(&buffer, sizeof(char), 2, bmp);   // Number of planes = 1
	buffer = 24;
	fwrite(&buffer, sizeof(char), 2, bmp);   // Number of bits per pixel
	buffer = 0;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Compression type (0 = none)
	buffer = width * height * 3;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Size of image data
	buffer = 0;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Horizontal resolution (pixels per meter)
	buffer = 0;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Vertical resolution
	buffer = 0;
	fwrite(&buffer, sizeof(char), 4, bmp);   // Number of colors and important colors
	fwrite(&buffer, sizeof(char), 4, bmp);
	// ---

	char * data = new char[width * height * 3];
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, data);

	fwrite(data, 3, width * height, bmp);

	delete data;
	fclose(bmp);

	return;
}

void Window::resize_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Window::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	switch (key) {
	case GLFW_KEY_ESCAPE:
		if (action != GLFW_RELEASE)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	case GLFW_KEY_P:
		if (action == GLFW_PRESS)
			saveScreenshot(window);
		break;
	}
}
