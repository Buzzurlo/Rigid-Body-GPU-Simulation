#include "Shader.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

Shader::Shader()
{
	program_id = glCreateProgram();
	usesTransformFeedback = false;
}

Shader::~Shader()
{
	printf("DELETING SHADER %d\n", program_id);
	glDeleteProgram(program_id);
}

void Shader::attach(GLenum type, const char * file)
{

	GLuint shader_id = glCreateShader(type);

	std::string shaderCode;
	std::ifstream shaderStream(file, std::ios::in);
	if (shaderStream.is_open())
	{
		std::string Line = "";
		while (getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", file);
		glDeleteShader(shader_id);
		return;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	char const * source = shaderCode.c_str();
	glShaderSource(shader_id, 1, &source, NULL);
	glCompileShader(shader_id);

	// Verifica
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		char* error_message = new char[InfoLogLength + 1];
		glGetShaderInfoLog(shader_id, InfoLogLength, NULL, error_message);
		printf("%s\n", error_message);
		delete error_message;
		glDeleteShader(shader_id);
		return;
	}

	glAttachShader(program_id, shader_id);
	printf("%s successfully attached\n", file);
}

void Shader::addTransformFeedback(int count, const char ** varyings, GLenum mode)
{
	glTransformFeedbackVaryings(program_id, count, varyings, mode);
	usesTransformFeedback = true;
}

void Shader::link()
{
	glLinkProgram(program_id);
	
	int log_length;
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		char* error_message = new char[log_length + 1];
		glGetProgramInfoLog(program_id, log_length, NULL, error_message);
		printf("%s\n", error_message);
		delete error_message;
		return;
	}

	int count;
	glGetProgramiv(program_id, GL_ATTACHED_SHADERS, &count);

	GLuint* shaders = new GLuint[count];
	glGetAttachedShaders(program_id, count, NULL, shaders);

	for (int i = 0; i < count; ++i) {
		glDetachShader(program_id, shaders[i]);
		glDeleteShader(shaders[i]);
	}
	delete shaders;
}

void Shader::bind()
{
	glUseProgram(program_id);
}

void Shader::unbind()
{
	glUseProgram(0);
}
