#pragma once

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include <vector>

#include "Mesh3.h"
#include "Shader.h"
#include "Particle.h"

using namespace glm;

struct RigidBodyInfo {
	vec3 position;
	float padding1;
	vec4 rotation;
	vec3 linearMomentum;
	float padding2;
	vec3 angularMomentum;
	float padding3;
	int num_particles;
	int offset;
	int padding4[2];
	float inv_mass;
	float padding5[3];
	mat4 inv_inertiaTensor;
};

class RigidBodyManager {
	Mesh3 model;
	float scale;
	float diam;
	
	vec3 * particles;
	int numParticles;
	float mass;
	float invMass;
	vec3 centerOfMass;
	mat3 inverseInertiaTensor;

	std::vector<RigidBodyInfo> bodies;

	mat3 computeInertiaTensor(float particle_mass);
public:
	RigidBodyManager(std::string filename, bool clockwise, float scale);
	~RigidBodyManager();

	int get_count();
	int get_num_particles();
	GLuint	buildBuffer();
	void add_bodies(int n, ParticleManager& pm);
	Mesh3 get_model();
	vec3 get_center();
	std::vector<vec3> get_particles();
	void compute_particles(float diameter);
};