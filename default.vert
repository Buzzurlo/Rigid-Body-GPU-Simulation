#version 400

layout (location = 0) in vec3 in_vertex;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpace;

out vec3 fragPos;
out vec3 fragNorm;
out vec3 fragColor;
out vec4 fragLightPos;

void main() {
	vec4 world_position = model * vec4(in_vertex, 1.0f);
	gl_Position = projection * view  * world_position;
	
	fragNorm = mat3(transpose(inverse(model))) * normalize(in_normal);
	fragColor = vec3(1.0f);
	fragPos = world_position.xyz;
	fragLightPos = lightSpace * model * vec4(in_vertex, 1.0f);
}