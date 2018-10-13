#version 430

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

layout(std430, binding = 1) buffer body_data {
	RigidBodyInfo bodies[];
};
layout(std430, binding = 2) buffer particle_data {
	ParticleInfo particles[];
};
layout(std430, binding = 3) buffer grid_data {
	int grid[256][256][256][20];
};

uniform float diam;

vec4 quat_mult(vec4 q, vec4 r) {
	vec4 res;
	res.w = r.w*q.w - r.x*q.x - r.y*q.y - r.z*q.z;
	res.x = r.w*q.x + r.x*q.w - r.y*q.z + r.z*q.y;
	res.y = r.w*q.y + r.x*q.z + r.y*q.w - r.z*q.x;
	res.z = r.w*q.z - r.x*q.y + r.y*q.x + r.z*q.w;
	return res;
}
vec4 quat_conj(vec4 q) {
	return vec4(-q.x, -q.y, -q.z, q.w);
}

vec3 rotate(vec3 v, vec4 q) {
	return quat_mult(q, quat_mult(vec4(v, 0.0f), quat_conj(q))).xyz;
}

void main() {
	ParticleInfo particle = particles[gl_VertexID];
	RigidBodyInfo body = bodies[particle.body];

	// if gl_VertexID >= particles.length() --> error
	
	particles[gl_VertexID].initialRelativePosition = particle.initialRelativePosition;
	particles[gl_VertexID].currentRelativePosition = rotate(particle.initialRelativePosition, body.rotation);
	particles[gl_VertexID].currentPosition = body.position + particles[gl_VertexID].currentRelativePosition;
	particles[gl_VertexID].currentVelocity = body.linearMomentum + cross(body.angularMomentum, particles[gl_VertexID].currentRelativePosition);
	particles[gl_VertexID].force = vec3(0);
	particles[gl_VertexID].body = particle.body;
	
	ivec3 ind = ivec3((particles[gl_VertexID].currentPosition - vec3(-diam * 128, 0, -diam * 128)) / diam);

	int offset = atomicAdd(grid[ind.x][ind.y][ind.z][0], 1);
	if(offset > 20)
		// error kinda, particle doesnt fit in grid, too many particles in the same cell, will skip collision
		atomicExchange(grid[ind.x][ind.y][ind.z][0], 20);
	else {
		++offset;
		atomicExchange(grid[ind.x][ind.y][ind.z][offset], gl_VertexID);
		particles[gl_VertexID].grid_index = ind;
		particles[gl_VertexID].grid_pos = offset;
	}
}