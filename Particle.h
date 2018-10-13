#pragma once

#include "glm\glm.hpp"
#include <vector>

#include "Mesh3.h"
#include "Shader.h"

using namespace glm;

struct ParticleInfo {
	vec3 initialRelativePosition;
	float padding1;
	vec3 currentRelativePosition;
	float padding2;
	vec3 currentPosition;
	float padding3;
	vec3 currentVelocity;
	float padding4;
	vec3 force;
	int body;
	ivec3 grid_index;
	int grid_pos;
};

class ParticleManager {
	Mesh3 sphere;
	Shader shader;
	std::vector<ParticleInfo> particles;

	int temp_n;

public:
	ParticleManager(float diam);

	Mesh3 get_model();
	Shader& get_shader();
	int get_count();

	GLuint buildBuffer();
	int add_particles(std::vector<ParticleInfo> data);
	void load_particles();
};