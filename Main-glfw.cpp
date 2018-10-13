#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL\glew.h>
#include <GL\glfw3.h>
#include "glm\glm.hpp"
#include "Renderer.h"
#include "Physics.h"

using namespace glm;

int RandInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}
float RandFloat(float min, float max) {
	return (float)rand() / RAND_MAX * (max - min) + min;
}

int main() {
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
	window = glfwCreateWindow(800, 600, "Physics Engine", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. Try the 2.1 version of openGL.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	srand(clock());
	const int n = 10;

	Physics phsx(n);
	for (int i = 0; i < n; i++) {
		phsx.AddObject(
			vec3(RandFloat(-9, 9),		RandFloat(-9, 9),		RandFloat(-9, 9)),
			vec3(RandFloat(-10, 10),	RandFloat(-10, 10),	RandFloat(-10, 10)),
			vec3(0, -9.81, 0)
		);
	}

	Renderer gfx;
	gfx.Init(window, phsx.GetPositionBuffer(), n);

	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	mat4 view = lookAt(
		vec3(2000, 2000, 2000),
		vec3(0, 0, 0),
		vec3(0, 1, 0));


	enum KEYS { W, A, S, D, Q, Z };
	bool keys[] = { false, false, false, false, false, false };
	bool spherical = false;
	double xAngle = 0, yAngle = 0, xPos = 1300, yPos = 100, zPos = 1300;
	double movSpeed = 500.0;
	double r = 200;
	if (!spherical)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	double frameTime, currentTime = glfwGetTime();

	do {
		double newTime = glfwGetTime();
		frameTime = newTime - currentTime;
		currentTime = newTime;

		// Draw nothing, see you in tutorial 2 !
		keys[W] = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
		keys[A] = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
		keys[S] = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
		keys[D] = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
		keys[Q] = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
		keys[Z] = (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS);

		if (keys[W] != keys[S]) {	// XOR
			if (keys[W]) {
				if (spherical)
					yAngle += PI / 10 / 60;
				else {
					xPos += sin(xAngle) * movSpeed * frameTime;
					zPos -= cos(xAngle) * movSpeed * frameTime;
				}
			}
			else {
				if (spherical)
					yAngle -= PI / 10 / 60;
				else {
					xPos -= sin(xAngle) * movSpeed * frameTime;
					zPos += cos(xAngle) * movSpeed * frameTime;
				}
			}
		}
		if (keys[A] != keys[D]) {
			if (keys[A]) {
				if (spherical)
					xAngle -= PI / 10 / 60;
				else {
					xPos -= sin(xAngle + PI / 2) * movSpeed * frameTime;
					zPos += cos(xAngle + PI / 2) * movSpeed * frameTime;
				}
			}
			else {
				if (spherical)
					xAngle += PI / 10 / 60;
				else {
					xPos -= sin(xAngle - PI / 2) * movSpeed * frameTime;
					zPos += cos(xAngle - PI / 2) * movSpeed * frameTime;
				}
			}
		}
		if (keys[Q]) {
			yPos += movSpeed * frameTime;
		}
		if (keys[Z]) {
			yPos -= movSpeed * frameTime;
		}

		if (!spherical) {
			double mx, my;
			glfwGetCursorPos(window, &mx, &my);
			xAngle = mx * 0.001 + 5;
			yAngle = my * 0.001 + 0.6;
		}
		float cx = r * cos(yAngle) * sin(xAngle);
		float cy = r * sin(yAngle);
		float cz = r * cos(yAngle) * cos(xAngle);

		if (spherical) {
			view = lookAt(vec3(cx, cy, cz), vec3(0, 0, 0), vec3(0, 1, 0));
		}
		else {
			view = lookAt(vec3(xPos, yPos, zPos),
				vec3(xPos + cx, yPos - cy, zPos - cz),
				vec3(0, 1, 0));
		}
		
		phsx.Update(frameTime);

		gfx.Render(view);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}