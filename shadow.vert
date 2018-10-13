#version 400

layout(location = 0) in vec3 in_vertex;

uniform mat4 model;
uniform mat4 lightSpace;

void main() {
	gl_Position = lightSpace * model * vec4(in_vertex, 1.0f);
}