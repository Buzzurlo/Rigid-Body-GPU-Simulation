#pragma once

#include <GL\glew.h>
#include <GL\glfw3.h>
#include "glm\glm.hpp"
#include "glm\gtx\transform.hpp"
#include "Mesh3.h"
#include "Shader.h"

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

class GPU_render {
	GLuint shaders;

	GLuint modelLoc, viewLoc, projLoc, viewPosLoc;

	GLuint depthMapFbo, depthMap;
	Shader shadowShaders, shadowShadersInstanced;
	GLuint texShaders;
	int res;

	GLFWwindow * window;
	int n_particles;
	int texSize;

public:
	// TODO : cleanup code and organize
	void Init(GLFWwindow * window, int n_particles);

	void Clear();
	void Render_but_better(mat4 view, mat4 model, Mesh3 mesh, Shader& shader, bool shadow);
	void Render_but_better_Instanced(mat4 view, mat4 model, Mesh3 mesh, Shader& shader, int n);

	void Render(mat4 view, Mesh3& object, Mesh3& walls, GLuint posTexture, GLuint velTexture);

	// destructor probably useless
	// program frees memory at the end anyway
	// only useful when the class is expected to have a short lifespan
	~GPU_render();
};