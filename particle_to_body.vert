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

layout (std430, binding = 1) buffer body_data {
	RigidBodyInfo bodies[];
};
layout (std430, binding = 2) buffer particle_data {
	ParticleInfo particles[];
};
layout (std430, binding = 3) buffer grid_data {
	int grid[256][256][256][20];
};

uniform float r;
uniform float part_density;
uniform int num_objects;

vec3 collision(ParticleInfo a, ParticleInfo b) {
	float r2 = (r + r)*(r + r);
	
	vec3 dir = b.currentPosition - a.currentPosition;
	float dist2 = dot(dir, dir);

	if (dist2 < r2 && a.body != b.body) {
		float x = r + r - length(dir);
		float k = 80000;
		float d = 10;
		float f = 25;

		dir = normalize(dir);
		vec3 fs = -k * x * dir;
		vec3 vel = a.currentVelocity - b.currentVelocity;
		vec3 fd = -d * vel;
		vec3 v_t = vel - dot(vel, dir) * dir;
		vec3 ft = -f * v_t;

		return (fs + fd + ft);
	}

	return vec3(0);
}

vec3 bounds_check(ParticleInfo a) {
	vec3 pos = a.currentPosition;
	float k = 1000;
	float d = 40;
	float f = 25;

	float x = 0;
	vec3 dir = vec3(0);

	if(pos.y - r < 0) {
		x = abs(pos.y - r);
		dir = vec3(0,1,0);
		vec3 fs = k * x * dir;
		vec3 fd = -d * dot(a.currentVelocity, dir) * dir;
		vec3 v_t = a.currentVelocity - dot(a.currentVelocity, dir) * dir;
		vec3 ft = -f * v_t;
		return (fs + fd + ft);
	}
	if(pos.x - r < -2*r*128) {
		x = abs(pos.x - r + 2*r*128);
		dir = vec3(1,0,0);
		vec3 fs = k * x * dir;
		vec3 fd = -d * dot(a.currentVelocity, dir) * dir;
		vec3 v_t = a.currentVelocity - dot(a.currentVelocity, dir) * dir;
		vec3 ft = -f * v_t;
		return (fs + fd + ft);
	}
	else if(pos.x + r > 2*r*128) {
		x = abs(pos.x + r - 2*r*128);
		dir = vec3(-1,0,0);
		vec3 fs = k * x * dir;
		vec3 fd = -d * dot(a.currentVelocity, dir) * dir;
		vec3 v_t = a.currentVelocity - dot(a.currentVelocity, dir) * dir;
		vec3 ft = -f * v_t;
		return (fs + fd + ft);
	}
	if(pos.z - r < -2*r*128) {
		x = abs(pos.z - r + 2*r*128);
		dir = vec3(0,0,1);
		vec3 fs = k * x * dir;
		vec3 fd = -d * dot(a.currentVelocity, dir) * dir;
		vec3 v_t = a.currentVelocity - dot(a.currentVelocity, dir) * dir;
		vec3 ft = -f * v_t;
		return (fs + fd + ft);
	}
	else if(pos.z + r > 2*r*128) {
		x = abs(pos.z + r - 2*r*128);
		dir = vec3(0,0,-1);
		vec3 fs = k * x * dir;
		vec3 fd = -d * dot(a.currentVelocity, dir) * dir;
		vec3 v_t = a.currentVelocity - dot(a.currentVelocity, dir) * dir;
		vec3 ft = -f * v_t;
		return (fs + fd + ft);
	}
	return vec3(0);
}

void main() {
	float part_mass =  4.188f * pow(r, 3) * part_density;
	vec3 force = vec3(0, -9.81f, 0);

	ParticleInfo A = particles[gl_VertexID];

	ivec3 ind = A.grid_index;
	int pos = A.grid_pos;
	int n = grid[ind.x][ind.y][ind.z][0];

	ivec3 min;
	min.x = ind.x > 0 ? -1 : 0;
	min.y = ind.y > 0 ? -1 : 0;
	min.z = ind.z > 0 ? -1 : 0;

	ivec3 max;
	max.x = ind.x < 255 ? 1 : 0;
	max.y = ind.y < 255 ? 1 : 0;
	max.z = ind.z < 255 ? 1 : 0;

	for(int x = min.x; x <= max.x; ++x)
		for(int y = min.y; y <= max.y; ++y)
			for(int z = min.z; z <= max.z; ++z)
				for(int p = 1; p <= n; ++p) {
					uint i = grid[ind.x + x][ind.y + y][ind.z + z][p];
					if(i == pos)
						continue;
					ParticleInfo B = particles[i];
					if(B.body >= num_objects)
						continue;
					force += collision(A, B);
				}

	particles[gl_VertexID].force = force + bounds_check(A) - 0.1 * A.currentVelocity;
	grid[ind.x][ind.y][ind.z][0] = 0;
}