#pragma once

#include <GL\glew.h>
#include <string>

class Shader {
public:
	Shader();
	~Shader();

	void attach(GLenum type, const char* file);
	void addTransformFeedback(int count, const char ** varyings, GLenum mode);
	void link();

	void bind();
	void unbind();

	GLuint get() { return program_id; }

private:
	GLuint program_id;
	bool usesTransformFeedback;
};