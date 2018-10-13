#pragma once

#include <GL\glew.h>
#include "glm\glm.hpp"

using namespace glm;

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

class GPU_impl {
private:
	GLuint fb;
	GLuint quad_vao, quad_vbo;
	GLuint pos[2], vel[2];
	GLuint physics_shader;
	GLuint dtLoc;

	int read, write;
	int texSize;

	static GLuint CreateTexture(int xsize, int ysize, void* data) {
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, xsize, ysize, 0, GL_RGBA, GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		return tex;
	}

public:
	GLuint GetCurrentPosTexture() {
		return pos[read];
	}
	GLuint GetCurrentVelTexture() {
		return vel[read];
	}

	void Init(int texSize, void* pos_data, void* vel_data) {
		read = 0;
		write = 1;
		this->texSize = texSize;

		glGenFramebuffers(1, &fb);
		glBindFramebuffer(GL_FRAMEBUFFER, fb);

		pos[read]	= CreateTexture(texSize, texSize, pos_data);
		pos[write]	= CreateTexture(texSize, texSize, 0);
		vel[read]	= CreateTexture(texSize, texSize, vel_data);
		vel[write]	= CreateTexture(texSize, texSize, 0);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffers);

		// ccw
		GLfloat quad[] = {
			-1.0f, -1.0f,
			 1.0f, -1.0f,
			-1.0f,  1.0f,

			-1.0f, -1.0f,
			 1.0f, -1.0f,
			 1.0f,  1.0f
		};

		glGenVertexArrays(1, &quad_vao);
		glBindVertexArray(quad_vao);

		glGenBuffers(1, &quad_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// - load correct shaders
		physics_shader = LoadShaders("passthrough.vert", "particle_update.frag");

		dtLoc = glGetUniformLocation(physics_shader, "dt");
	}

	void Compute(float frameTime) {
		glBindFramebuffer(GL_FRAMEBUFFER, fb);

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_MULTISAMPLE);
		glFrontFace(GL_CCW);

		glUseProgram(physics_shader);

		// inputs
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pos[read]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, vel[read]);

		// uniforms
		// can be optimized by saving the location
		glUniform1i(glGetUniformLocation(physics_shader, "posTexture"), 0);
		glUniform1i(glGetUniformLocation(physics_shader, "velTexture"), 1);
		glUniform1i(glGetUniformLocation(physics_shader, "texSize"), texSize);
		// like this
		glUniform1f(dtLoc, frameTime);

		// outputs
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pos[write], 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, vel[write], 0);

		glBindVertexArray(quad_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// swap
		// here the output is "write"
		read = 1 - read;
		write = 1 - read;
		// here the output is "read"
	}
};