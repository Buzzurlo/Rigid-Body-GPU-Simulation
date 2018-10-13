#include "GPU_render.h"

void GPU_render::Init(GLFWwindow * window, int n_particles) {
	this->window = window;
	this->n_particles = n_particles;
	texSize = sqrt(n_particles);

	shaders = LoadShaders("gpu_shader.vert", "gpu_shader.frag");

	modelLoc = glGetUniformLocation(shaders, "model");
	viewLoc = glGetUniformLocation(shaders, "view");
	projLoc = glGetUniformLocation(shaders, "projection");

	res = 2048;
	
	glGenFramebuffers(1, &depthMapFbo);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, res, res, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//shadowShaders = LoadShaders("shadow.vert", "shadow.frag");
	shadowShaders.attach(GL_VERTEX_SHADER, "shadow.vert");
	shadowShaders.attach(GL_FRAGMENT_SHADER, "shadow.frag");
	shadowShaders.link();

	shadowShadersInstanced.attach(GL_VERTEX_SHADER, "shadow_instanced.vert");
	shadowShadersInstanced.attach(GL_FRAGMENT_SHADER, "shadow.frag");
	shadowShadersInstanced.link();

	texShaders = LoadShaders("texture_draw.vert", "texture_draw.frag");

	// V-SYNC
	glfwSwapInterval(1);
}

void GPU_render::Clear() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
	glClear(GL_DEPTH_BUFFER_BIT);
}
void GPU_render::Render_but_better(mat4 view, mat4 model, Mesh3 mesh, Shader& shader, bool shadow) {
	mat4 lightSpace = mat4(0);

	if (shadow) {
		glViewport(0, 0, res, res);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
		//glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glFrontFace(mesh.winding);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		mat4 lightProjection = perspective(90.0f, 1.0f, 0.1f, 200.0f);
		mat4 lightView = lookAt(vec3(20, 20, 20), vec3(0, 0, 0), vec3(0, 1, 0));
		lightSpace = lightProjection * lightView;
		shadowShaders.bind();
		glUniformMatrix4fv(glGetUniformLocation(shadowShaders.get(), "model"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadowShaders.get(), "lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);

		mesh.bind();
		mesh.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	shader.bind();

	glEnable(GL_CULL_FACE);
	//glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glDepthFunc(GL_LESS);
	glFrontFace(mesh.winding);

	mat4 projection = perspective(radians(75.0f), 800.0f/600.0f, 0.1f, 200.0f);
	
	//printf("error: 0x%x\n", glGetError());
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(shader.get(), "shadowMap"), 0);

	mesh.bind();
	mesh.draw();

	
	if (shadow) {
		// debug
		GLfloat quad[] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			-1.0f,  1.0f,

			-1.0f,  1.0f,
			1.0f, -1.0f,
			1.0f,  1.0f,
		};
		GLuint quad_vao, quad_vbo;
		glGenVertexArrays(1, &quad_vao);
		glBindVertexArray(quad_vao);

		glGenBuffers(1, &quad_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// draw depth textures
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(580, 380, 200, 200);
		//glDisable(GL_DEPTH_TEST);
		glUseProgram(texShaders);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(glGetUniformLocation(texShaders, "depthTextures"), 0);
		glFrontFace(GL_CCW);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteVertexArrays(1, &quad_vao);
		glDeleteBuffers(1, &quad_vbo);
	}

	GLenum error = glGetError();
	if(error != GL_NO_ERROR)
		printf("error: 0x%x\n", error);
}
void GPU_render::Render_but_better_Instanced(mat4 view, mat4 model, Mesh3 mesh, Shader& shader, int n) {
	mat4 lightSpace = mat4(1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	glEnable(GL_CULL_FACE);		// big optimizer
	glEnable(GL_DEPTH_TEST);	// medium optimizer
	glEnable(GL_MULTISAMPLE);
	glDepthFunc(GL_LESS);
	glFrontFace(mesh.winding);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	mat4 projection = perspective(radians(75.0f), (float)width / height, 0.1f, 200.0f);
	//view = lookAt(vec3(1000, 1000, 1000), vec3(0, 500, 0), vec3(0, 1, 0));\

	// uniforms
	shader.bind();
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader.get(), "lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(shader.get(), "shadowMap"), 0);

	mesh.bind();
	mesh.draw_instanced(n);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		printf("error: 0x%x\n", error);
}

void GPU_render::Render(mat4 view, Mesh3& object, Mesh3& walls, GLuint posTexture, GLuint velTexture) {
	glViewport(0, 0, res, res);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
	//glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	glFrontFace(object.winding);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	mat4 lightProjection = perspective(90.0f, 1.0f, 0.1f, 200.0f);
	mat4 lightView = lookAt(vec3(25, 20, 25), vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 lightSpace = lightProjection * lightView;
	mat4 model = scale(vec3(1));
	shadowShadersInstanced.bind();
	glUniformMatrix4fv(glGetUniformLocation(shadowShaders.get(), "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadowShaders.get(), "lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, posTexture);
	glUniform1i(glGetUniformLocation(shadowShadersInstanced.get(), "centers"), 0);
	glUniform1i(glGetUniformLocation(shadowShadersInstanced.get(), "texSize"), texSize);

	object.bind();
	object.draw_instanced(n_particles);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	glUseProgram(shaders);

	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);		// big optimizer
	glEnable(GL_DEPTH_TEST);	// medium optimizer
	glEnable(GL_MULTISAMPLE);
	glDepthFunc(GL_LESS);
	glFrontFace(object.winding);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	mat4 projection = perspective(radians(75.0f), (float)width / height, 0.1f, 200.0f);
	//mat4 model = scale(vec3(1));

	// uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
	glUniform1i(glGetUniformLocation(shaders, "texSize"), texSize);
	glUniformMatrix4fv(glGetUniformLocation(shaders, "lightSpace"), 1, GL_FALSE, &lightSpace[0][0]);


	if (posTexture >= 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, posTexture);
		glUniform1i(glGetUniformLocation(shaders, "centers"), 0);
	}
	if (velTexture >= 0) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, velTexture);
		glUniform1i(glGetUniformLocation(shaders, "vel"), 1);
	}
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(shaders, "shadowMap"), 2);

	object.bind();
	object.draw_instanced(n_particles);

	//model = translate(vec3(0, -10, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
	glFrontFace(GL_CCW);
	walls.bind();
	walls.draw();

	/*
	GLfloat quad[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,

		-1.0f,  1.0f,
		1.0f, -1.0f,
		1.0f,  1.0f,
	};
	GLuint quad_vao, quad_vbo;
	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);

	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// draw depth textures
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(580, 380, 200, 200);
	//glDisable(GL_DEPTH_TEST);
	glUseProgram(texShaders);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(texShaders, "depthTextures"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &quad_vao);
	glDeleteBuffers(1, &quad_vbo);
	*/

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		printf("error: 0x%x\n", error);
}

// destructor probably useless
// program frees memory at the end anyway
// only useful when the class is expected to have a short lifespan
GPU_render::~GPU_render() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glDeleteProgram(shaders);
}