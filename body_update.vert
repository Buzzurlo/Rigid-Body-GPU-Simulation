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

uniform float dt;

mat3 quat_to_matrix(vec4 q) {
	mat3 res;
	res[0] = vec3(1 - 2*q.y*q.y - 2*q.z*q.z,	2*q.x*q.y + 2*q.z*q.w,		2*q.x*q.z - 2*q.y*q.w);
	res[1] = vec3(2*q.x*q.y - 2*q.z*q.w,		1 - 2*q.x*q.x - 2*q.z*q.z,	2*q.y*q.z + 2*q.x*q.w);
	res[2] = vec3(2*q.x*q.z + 2*q.y*q.w,		2*q.y*q.z - 2*q.x*q.w,		1 - 2*q.x*q.x - 2*q.y*q.y);
	return res;
}

vec4 quat_mult(vec4 q, vec4 r) {
	vec4 res;
	res.w = r.w*q.w - r.x*q.x - r.y*q.y - r.z*q.z;
	res.x = r.w*q.x + r.x*q.w - r.y*q.z + r.z*q.y;
	res.y = r.w*q.y + r.x*q.z + r.y*q.w - r.z*q.x;
	res.z = r.w*q.z - r.x*q.y + r.y*q.x + r.z*q.w;
	return res;
}

vec4 quat_mult2(vec4 q0, vec4 q1) {
	vec4 res;
	res.w = q0.w * q1.w - dot(q0.xyz, q1.xyz);
	res.xyz = q0.w * q1.xyz + q1.w * q0.xyz + cross(q0.xyz, q1.xyz);
	return res;
}

void main() {
	RigidBodyInfo body = bodies[gl_VertexID];
	vec3 force = vec3(0);
	vec3 torque = vec3(0);

	for(int i = 0; i < body.num_particles; ++i) {
		force += particles[i + body.offset].force;
		torque += cross(particles[i + body.offset].currentRelativePosition, particles[i + body.offset].force);
	}

	bodies[gl_VertexID].linearMomentum += force * body.inv_mass * dt;
	bodies[gl_VertexID].position += bodies[gl_VertexID].linearMomentum * dt;

	
	mat3 rot = quat_to_matrix(body.rotation);
	mat3 curr_inv_inertiaTensor = rot * mat3(body.inv_inertiaTensor) * transpose(rot);
	bodies[gl_VertexID].angularMomentum += curr_inv_inertiaTensor * torque * dt;

	vec3 a = normalize(bodies[gl_VertexID].angularMomentum);
	float th = length(bodies[gl_VertexID].angularMomentum * dt);
	vec4 dq = vec4(a * sin(th / 2), cos(th / 2));
	bodies[gl_VertexID].rotation = quat_mult2(dq, body.rotation);
}