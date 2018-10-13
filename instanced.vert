#version 400

layout (location = 0) in vec3 in_vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;
layout (location = 3) in vec3 in_center;
layout (location = 4) in vec4 in_rotation;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpace;

out vec3 fragPos;
out vec3 fragNorm;
out vec3 fragColor;
out vec4 fragLightPos;

vec3 colors[] = {
	vec3(1,0,0),
	vec3(0,1,0),
	vec3(0,0,1),
	vec3(1,0,1),
	vec3(1,1,0),
	vec3(0,1,1),
	vec3(1,1,1),
	vec3(0,0,0)
};

mat3 quat_to_matrix(vec4 q) {
	mat3 res;
	res[0] = vec3(1 - 2*q.y*q.y - 2*q.z*q.z,	2*q.x*q.y + 2*q.z*q.w,		2*q.x*q.z - 2*q.y*q.w);
	res[1] = vec3(2*q.x*q.y - 2*q.z*q.w,		1 - 2*q.x*q.x - 2*q.z*q.z,	2*q.y*q.z + 2*q.x*q.w);
	res[2] = vec3(2*q.x*q.z + 2*q.y*q.w,		2*q.y*q.z - 2*q.x*q.w,		1 - 2*q.x*q.x - 2*q.y*q.y);
	return res;
}

void main() {
	mat3 rot = quat_to_matrix(in_rotation);
	vec3 world_position = rot * vec3(model * vec4(in_vertex, 1.0f)) + in_center;
	gl_Position = projection * view  * vec4(world_position, 1.0f);
	
	fragNorm = mat3(transpose(inverse(model * mat4(rot)))) * normalize(in_normal);
	fragColor = colors[gl_InstanceID % 8];
	fragPos = world_position;
	fragLightPos = lightSpace * vec4(world_position, 1.0f);
}