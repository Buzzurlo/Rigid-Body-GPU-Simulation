#version 430

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

layout (std430, binding = 2) buffer particle_data {
	ParticleInfo particles[];
};
layout (std430, binding = 3) buffer grid_data {
	int grid[256][256][256][21];
};

uniform float diam;

void main() {
	ivec3 ind = ivec3((particles[gl_VertexID].currentPosition - vec3(-diam * 128, 0, -diam * 128)) / diam);

	int offset = atomicAdd(grid[ind.x][ind.y][ind.z][0], 1);
	if(offset > 20)
		// error kinda, particle doesnt fit in grid, too many particles in the same cell, will skip collision
		atomicMin(grid[ind.x][ind.y][ind.z][0], 20);
	else {
		++offset;
		atomicExchange(grid[ind.x][ind.y][ind.z][offset], gl_VertexID);
		particles[gl_VertexID].grid_index = ind;
		particles[gl_VertexID].grid_pos = offset;
	}
}