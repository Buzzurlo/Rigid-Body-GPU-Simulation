#include "RigidBody.h"

#include "glm\gtx\transform.hpp"

using namespace glm;

// helper
float randf(float min, float max) {
	return (float)rand() / RAND_MAX * (max - min) + min;
}
vec3 clamp(vec3 val, vec3 min, vec3 max) {
	val.x = clamp(val.x, min.x, max.x);
	val.y = clamp(val.y, min.y, max.y);
	val.z = clamp(val.z, min.z, max.z);
	return val;
}

int RigidBodyManager::get_count()
{
	return bodies.size();
}

int RigidBodyManager::get_num_particles()
{
	return numParticles;
}

GLuint RigidBodyManager::buildBuffer()
{
	GLuint dataBuffer;
	glGenBuffers(1, &dataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, dataBuffer);
	// use this later, for variable body count
	// glBufferData(GL_ARRAY_BUFFER, sizeof(RigidBodyInfo) * MAX_BODIES, NULL, GL_DYNAMIC_COPY);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(RigidBodyInfo) * bodies.size(), bodies.data(), GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dataBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	return dataBuffer;
}

mat3 RigidBodyManager::computeInertiaTensor(float particle_mass)
{
	mat3 res = mat3(0);
	for (int i = 0; i < numParticles; ++i) {
		vec3 p = particles[i];
		res += 1.0f * (dot(p, p) * mat3(1) - outerProduct(p, p));
	}
	return res;
}

RigidBodyManager::RigidBodyManager(std::string filename, bool clockwise, float scale)
{
	this->scale = scale;
	model.load(filename, clockwise);
	model.save_changes();
}

RigidBodyManager::~RigidBodyManager()
{
	if (particles)
		delete particles;
}

void RigidBodyManager::add_bodies(int n, ParticleManager& pm)
{
	for (int i = 0; i < n; ++i) {
		RigidBodyInfo body_info;
		body_info.position = vec3(randf(-5, 5), randf(i + 5, i + 15), randf(-5, 5));
		body_info.position = vec3(0, 15, 0);
		body_info.rotation = vec4(0, 0, 0, 1);
		body_info.linearMomentum  = vec3(randf(-2, 2), randf(-2, 2), randf(-2, 2));
		body_info.linearMomentum = vec3(0);
		body_info.angularMomentum = vec3(randf(-1, 1), randf(-1, 1), randf(-1, 1));
		body_info.inv_mass = invMass;
		body_info.inv_inertiaTensor = mat4(inverseInertiaTensor);

		// add particles
		std::vector<ParticleInfo> part_data;
		for (int p = 0; p < numParticles; ++p) {
			ParticleInfo part_info;
			part_info.initialRelativePosition = particles[p];
			part_info.currentRelativePosition = particles[p];
			part_info.currentPosition = body_info.position + particles[p];
			part_info.currentVelocity = body_info.linearMomentum + cross(body_info.angularMomentum, particles[p]);
			part_info.force = vec3(0);
			part_info.body = i;
			part_info.grid_index = ivec3((part_info.currentPosition - vec3(-diam * 128, 0, -diam * 128)) / diam);
			part_info.grid_pos = 1;
			part_data.push_back(part_info);
		}

		body_info.num_particles = numParticles;
		body_info.offset = pm.add_particles(part_data);
		bodies.push_back(body_info);
	}
}

Mesh3 RigidBodyManager::get_model()
{
	return model;
}

vec3 RigidBodyManager::get_center()
{
	return centerOfMass;
}

std::vector<vec3> RigidBodyManager::get_particles()
{
	std::vector<vec3> output(numParticles);
	output.insert(output.begin(), particles, particles + numParticles);
	return output;
}

void RigidBodyManager::compute_particles(float diameter) {
	diam = diameter;		// diameter of particle
	vec3 min, max;		// AABB for model
	model.compute_aabb(min, max);

	mat4 view = lookAt(vec3(0, 0, max.z), vec3(0, 0, (min.z + max.z) / 2), vec3(0, 1, 0));
	mat4 projection = ortho(min.x, max.x, min.y, max.y, max.z - min.z, 0.0f);

	float scale_factor = scale;		// model dependent
	min *= scale_factor;
	max *= scale_factor;

	// dimensions of 3D texture (number of cells)
	int w = (max.x - min.x) / diam + 1;
	int h = (max.y - min.y) / diam + 1;
	int d = (max.z - min.z) / diam + 1;
	// ivec3 dim = (max - min) / diam + ivec3(1);
	//								  + 1;


	Shader depth_shaders, tex_shaders, tf_shaders;
	depth_shaders.attach(GL_VERTEX_SHADER, "depth_peel.vert");
	depth_shaders.attach(GL_FRAGMENT_SHADER, "depth_peel.frag");
	depth_shaders.link();

	tex_shaders.attach(GL_VERTEX_SHADER, "depth_peel.vert");
	tex_shaders.attach(GL_FRAGMENT_SHADER, "depth_peel.frag");
	tex_shaders.link();

	tf_shaders.attach(GL_VERTEX_SHADER, "tf.vert");
	tf_shaders.attach(GL_GEOMETRY_SHADER, "tf.geom");
	const char* varyings[1] = { "out_pos" };
	tf_shaders.addTransformFeedback(1, varyings, GL_INTERLEAVED_ATTRIBS);
	tf_shaders.link();

	// depth peel once to know how many layers
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	GLuint color_renderbuffer, depth_renderbuffer;
	glGenRenderbuffers(1, &color_renderbuffer);
	glGenRenderbuffers(1, &depth_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RED, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

	glViewport(0, 0, w, h);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	depth_shaders.bind();

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
	glDepthFunc(GL_LESS);

	glUniformMatrix4fv(glGetUniformLocation(depth_shaders.get(), "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(depth_shaders.get(), "projection"), 1, GL_FALSE, &projection[0][0]);

	glFrontFace(GL_CW);
	model.bind();

	// query to know if there should be a next layer
	GLuint occl_query;
	glGenQueries(1, &occl_query);
	glBeginQuery(GL_ANY_SAMPLES_PASSED, occl_query);
	model.draw();
	glEndQuery(GL_ANY_SAMPLES_PASSED);

	int end, i = 1;
	glGetQueryObjectiv(occl_query, GL_QUERY_RESULT, &end);	// GL_TRUE or GL_FALSE

															// draw successive depth layers
	glDepthFunc(GL_GREATER);
	while (end == GL_TRUE) {
		glClear(GL_COLOR_BUFFER_BIT);
		glBeginQuery(GL_ANY_SAMPLES_PASSED, occl_query);
		model.draw();
		glEndQuery(GL_ANY_SAMPLES_PASSED);
		glGetQueryObjectiv(occl_query, GL_QUERY_RESULT, &end);
		++i;
	}

	// create 2d texture array of the right size
	// and depth peel again
	GLuint depth_imgs;
	glGenTextures(1, &depth_imgs);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depth_imgs);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, w, h, i - 1, 0, GL_RED, GL_FLOAT, NULL);
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, w, h, i - 1);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, depth_imgs, 0, 0);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depth_imgs, 0, 0);

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);
	glDepthFunc(GL_LESS);

	glUniformMatrix4fv(glGetUniformLocation(depth_shaders.get(), "view"), 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(depth_shaders.get(), "projection"), 1, GL_FALSE, &projection[0][0]);

	glFrontFace(GL_CW);

	model.bind();
	model.draw();

	// draw successive depth layers
	glDepthFunc(GL_GREATER);
	for (int layer = 1; layer < i; ++layer) {
		//glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, depth_imgs, 0, layer);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depth_imgs, 0, layer);
		glClear(GL_COLOR_BUFFER_BIT);
		model.draw();
	}

	// setup fullscreen quad
	//  used to draw deph textures
	//  !!! only for debugging, can remove in final code !!!
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
	tex_shaders.bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depth_imgs);
	glUniform1i(glGetUniformLocation(tex_shaders.get(), "depthTextures"), 0);
	glUniform1i(glGetUniformLocation(tex_shaders.get(), "layer"), 0);
	glViewport(0, 300, 800, 300);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUniform1i(glGetUniformLocation(tex_shaders.get(), "layer"), 1);
	glViewport(0, 0, 800, 300);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	printf("error: 0x%x\n", glGetError());
	// end of depth texture render, remove until here

	// filter the particles inside the model
	// using transform feedback and geometry shader

	// create shader program
	tf_shaders.bind();

	// vao with no inputs
	GLuint empty_vao;
	glGenVertexArrays(1, &empty_vao);
	glBindVertexArray(empty_vao);

	// create tf buffer and alloc
	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * w * h * d, nullptr, GL_STATIC_READ);

	// setup transform feedback
	glEnable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

	// query the number of elements written to tf
	GLuint query;
	glGenQueries(1, &query);

	// attach depth textures (2d texture array)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depth_imgs);
	glUniform1i(glGetUniformLocation(tf_shaders.get(), "depthTextures"), 0);

	// compute
	glUniform3i(glGetUniformLocation(tf_shaders.get(), "dimensions"), w, h, d);
	glUniform3fv(glGetUniformLocation(tf_shaders.get(), "max"), 1, &max[0]);
	glUniform3fv(glGetUniformLocation(tf_shaders.get(), "min"), 1, &min[0]);
	glUniform1f(glGetUniformLocation(tf_shaders.get(), "diam"), diam);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, w * h * d);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glDisable(GL_RASTERIZER_DISCARD);

	// get n of elements filtered
	GLuint n;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &n);
	particles = new vec3[n];
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, n * sizeof(vec3), particles);

	numParticles = n;
	float r = diam / 2;
	float volume = 4.0f / 3 * pi<float>() * r * r * r;
	float particle_density = 2;		// water
	mass = n * volume * particle_density;
	mass = numParticles;
	invMass = 1.0 / mass;
	centerOfMass = vec3(0);
	for (int i = 0; i < n; ++i)
		centerOfMass += particles[i];
	centerOfMass /= n;
	for (int i = 0; i < n; ++i)
		particles[i] -= centerOfMass;

	inverseInertiaTensor = inverse(computeInertiaTensor(volume * particle_density));
}
