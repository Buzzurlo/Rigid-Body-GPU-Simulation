#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL\glew.h>
#include <GL\glfw3.h>
#include "glm\glm.hpp"
#include "GPU_impl.h"
#include "GPU_render.h"
#include "Window.h"
#include "Camera.h"
#include "Particle.h"
#include "RigidBody.h"

using namespace glm;

// globals
float diameter = 0.125;
int num_objects = 0;


// functions
int				RandInt (int min, int max);
float			RandFloat (float min, float max);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		num_objects = min(num_objects + 1, 300);
		printf("\t%d", num_objects);
	}
	if(key == GLFW_KEY_ESCAPE && action != GLFW_RELEASE)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}


class Engine {
public:
	GLuint body_buffer, particle_buffer, grid_buffer, empty_vao;
	Shader btp, ptb, bu;

public:
	void init(RigidBodyManager& rbm, ParticleManager& pm) {
		body_buffer = rbm.buildBuffer();
		particle_buffer = pm.buildBuffer();

		glGenBuffers(1, &grid_buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, grid_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * 256 * 256 * 21 * sizeof(int), NULL, GL_DYNAMIC_COPY);
		int clear[4] = { 0,0,0,0 };
		glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_RGBA32I, GL_RGBA, GL_INT, clear);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grid_buffer);

		glGenVertexArrays(1, &empty_vao);

		btp.attach(GL_VERTEX_SHADER, "body_to_particle.vert");
		btp.link();
		ptb.attach(GL_VERTEX_SHADER, "particle_to_body.vert");
		ptb.link();
		bu.attach(GL_VERTEX_SHADER, "body_update.vert");
		bu.link();

		// draw shader

		rbm.get_model().bind();
		glBindBuffer(GL_ARRAY_BUFFER, body_buffer);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(RigidBodyInfo), 0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(RigidBodyInfo), (void*) sizeof(vec4));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);

		pm.get_model().bind();
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*) (sizeof(vec4) * 2));
		glVertexAttribDivisor(3, 1);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
			printf("error: 0x%x\n", error);
	}

	void update(RigidBodyManager& rbm, ParticleManager& pm, float dt) {
		GLenum error;
		// first pass: get particle values and insert into grid
		glEnable(GL_RASTERIZER_DISCARD);
		
		btp.bind();
		glBindVertexArray(empty_vao);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, body_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particle_buffer);
		
		glUniform1f(glGetUniformLocation(btp.get(), "diam"), diameter);
		//glDrawArrays(GL_POINTS, 0, pm.get_count());
		glDrawArrays(GL_POINTS, 0, num_objects * rbm.get_num_particles());

		ptb.bind();
		glBindVertexArray(empty_vao);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, body_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particle_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grid_buffer);
		float r = diameter / 2;
		glUniform1f(glGetUniformLocation(ptb.get(), "r"), r);
		glUniform1f(glGetUniformLocation(ptb.get(), "part_density"), 2);
		glUniform1i(glGetUniformLocation(ptb.get(), "num_objects"), num_objects);

		//glDrawArrays(GL_POINTS, 0, pm.get_count());
		glDrawArrays(GL_POINTS, 0, num_objects * rbm.get_num_particles());

		/*
		RigidBodyInfo* bodies = (RigidBodyInfo*)glMapNamedBuffer(body_buffer, GL_READ_ONLY);
		while ((error = glGetError()) != GL_NO_ERROR) printf("0x%x\n", error);
		RigidBodyInfo body = bodies[0];
		body = bodies[1];
		body = bodies[2];
		body = bodies[3];
		glUnmapNamedBuffer(body_buffer);

		ParticleInfo* particles = (ParticleInfo*)glMapNamedBuffer(particle_buffer, GL_READ_ONLY);
		while ((error = glGetError()) != GL_NO_ERROR) printf("0x%x\n", error);
		ParticleInfo particle = particles[0];
		particle = particles[700];
		particle = particles[24];
		particle = particles[25];
		glUnmapNamedBuffer(particle_buffer);
		*/
		/*
		ivec4* grid = (ivec4*)glMapNamedBuffer(grid_buffer, GL_READ_ONLY);
		while ((error = glGetError()) != GL_NO_ERROR) printf("0x%x\n", error);
		ivec4 cell = grid[0];
		cell = grid[256 * 128 + 128];
		cell = grid[256 * 256 * 256 - 1];
		cell = grid[25];
		glUnmapNamedBuffer(grid_buffer);
		*/

		bu.bind();
		glBindVertexArray(empty_vao);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, body_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particle_buffer);
		glUniform1f(glGetUniformLocation(bu.get(), "dt"), dt);
		//glDrawArrays(GL_POINTS, 0, rbm.get_count());
		glDrawArrays(GL_POINTS, 0, num_objects);

		glDisable(GL_RASTERIZER_DISCARD);

	}
};


int main() {
	Window window("Physics Engine", 800, 600, false);
	glfwSetKeyCallback(window.get(), key_callback);

	//srand(clock());
	srand(1);

	GPU_impl physics;

	// number of particles is texSize ^ 2
	const int texSize = 64;
	vec4 pos[texSize * texSize], vel[texSize * texSize];
	for (int i = 0; i < texSize * texSize; ++i) {
		pos[i] = vec4(RandFloat(-19, 19), RandFloat(-19, 19), RandFloat(-19, 19), 1.0f);
		vel[i] = vec4(RandFloat(-20, 20), RandFloat(-20, 20), RandFloat(-20, 20), 1.0f);
	}

	printf("Number of particles: %d\n", texSize*texSize);

	physics.Init(texSize, pos, vel);

	GPU_render graphics;
	graphics.Init(window.get(), texSize * texSize);

	Mesh3 walls;
	walls.add_plane(
		vec3(0, 1, 0),
		vec3(1, 0, 0),
		vec3(0, 0, 0),
		diameter * 128);
	//walls.add_cube(vec3(-diameter * 128, diameter * 256, -diameter * 128), vec3(diameter * 256));
	walls.save_changes();

	Mesh3 lights;
	lights.add_sphere(vec3(20, 20, 20), 1);
	lights.add_sphere(vec3(0, 20, 20), 1);
	lights.add_sphere(vec3(10, 20, 0), 1);
	lights.save_changes();

	ParticleManager pm(diameter);
	RigidBodyManager armchairs("cat_tri.obj", false, 1);
	armchairs.compute_particles(diameter);
	armchairs.add_bodies(300, pm);
	pm.load_particles();

	Engine engine;
	engine.init(armchairs, pm);

	Camera camera;
	//camera.setPosition(10, 20, 40);
	camera.setPosition(10, 10, 20);
	camera.point(vec3(0, 0, 0));
	
	Shader shader;
	shader.attach(GL_VERTEX_SHADER, "default.vert");
	shader.attach(GL_FRAGMENT_SHADER, "gpu_shader.frag");
	shader.link();

	printf("\n\n\n\tmax objects: 300\n");
	printf("\tparticles per objects: %d\n", armchairs.get_num_particles());
	printf("\tmax particles: %d\n\n\n", pm.get_count());

	enum KEYS { W, A, S, D, Q, Z };
	bool keys[] = { false, false, false, false, false, false };
	double moveSpeed = 10.0;
	double mouse_sens = 0.001;

	double frameTime, currentTime = glfwGetTime(), totalTime = 0;
	int frames = 0;

	do {
		double newTime = glfwGetTime();
		frameTime = newTime - currentTime;
		currentTime = newTime;
		totalTime += frameTime;

		// Draw nothing, see you in tutorial 2 !
		keys[W] = (glfwGetKey(window.get(), GLFW_KEY_W) == GLFW_PRESS);
		keys[A] = (glfwGetKey(window.get(), GLFW_KEY_A) == GLFW_PRESS);
		keys[S] = (glfwGetKey(window.get(), GLFW_KEY_S) == GLFW_PRESS);
		keys[D] = (glfwGetKey(window.get(), GLFW_KEY_D) == GLFW_PRESS);
		keys[Q] = (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS);
		keys[Z] = (glfwGetKey(window.get(), GLFW_KEY_Z) == GLFW_PRESS);

		if (keys[W]) camera.moveRelative( moveSpeed * frameTime, vec3(0, 0, -1));
		if (keys[S]) camera.moveRelative(-moveSpeed * frameTime, vec3(0, 0, -1));
		if (keys[A]) camera.moveRelative(-moveSpeed * frameTime, vec3(1, 0,  0));
		if (keys[D]) camera.moveRelative( moveSpeed * frameTime, vec3(1, 0,  0));
		if (keys[Q]) camera.moveAxis( moveSpeed * frameTime, vec3(0, 1, 0));
		if (keys[Z]) camera.moveAxis(-moveSpeed * frameTime, vec3(0, 1, 0));

		// to be changed with input handler
		double x_angle, y_angle;
		glfwGetCursorPos(window.get(), &x_angle, &y_angle);
		x_angle *= -mouse_sens;
		y_angle *= -mouse_sens;
		//camera.fromEulerAngles(x_angle, y_angle, 0.01 * sin(3 * frames * PI / 50.0));
		camera.fromEulerAngles(x_angle, y_angle, 0);
		
		//physics.Compute(frameTime);
		engine.update(armchairs, pm, frameTime);

		/*
		//bodies[0].force += vec3(0.01, 0, 0);
		printf("position: %.2f %.2f %.2f\n", body.position.x, body.position.y, body.position.z);
		printf("rotation: %.2f %.2f %.2f %.2f\n", body.rotation.x, body.rotation.y, body.rotation.z, body.rotation.w);
		printf("velocity: %.2f %.2f %.2f\n", body.linearMomentum.x, body.linearMomentum.y, body.linearMomentum.z);
		printf("angular : %.2f %.2f %.2f\n", body.angularMomentum.x, body.angularMomentum.y, body.angularMomentum.z);
		printf("force   : %.2f %.2f %.2f\n", body.force.x, body.force.y, body.force.z);
		printf("torque  : %.2f %.2f %.2f\n\n", body.torque.x, body.torque.y, body.torque.z);
		//getchar();
		*/
		
		//getchar();

		graphics.Clear();
		//graphics.Render_but_better_Instanced(camera.getViewMatrix(), translate(-armchairs.get_center()), armchairs.get_model(), pm.get_shader(), armchairs.get_count());
		graphics.Render_but_better_Instanced(camera.getViewMatrix(), translate(-armchairs.get_center()), armchairs.get_model(), pm.get_shader(), num_objects);
		//graphics.Render_but_better(camera.getViewMatrix(), scale(vec3(5)), armchairs.get_model(), shader, true);
		graphics.Render_but_better(camera.getViewMatrix(), scale(vec3(1)), walls, shader, false);
		//graphics.Render_but_better(camera.getViewMatrix(), scale(vec3(1)), lights, shader, false);

		// bugged!!! probably something related to instanced buffers
		//graphics.Render_but_better_Instanced(camera.getViewMatrix(), mat4(1), pm.get_model(), pm.get_shader(), pm.get_count());
		//graphics.Render_but_better_Instanced(camera.getViewMatrix(), mat4(1), pm.get_model(), pm.get_shader(), num_objects * armchairs.get_num_particles());

		//graphics.Render(camera.getViewMatrix(), pm.get_model(), walls, physics.GetCurrentPosTexture(), physics.GetCurrentVelTexture());

		if (frames == 100) {
			//printf("fps: %.2f\n", frames / totalTime);
			frames = 0;
			totalTime = 0;
		}
		++frames;

		//GLenum error = glGetError();
		//if (error != GL_NONE)
		//	printf("error: 0x%x\n", error);

		window.update();
	}
	while (!window.closed());

	return 0;
}

int RandInt(int min, int max) {
	return rand() % (max - min + 1) + min;
}
float RandFloat(float min, float max) {
	return (float)rand() / RAND_MAX * (max - min) + min;
}