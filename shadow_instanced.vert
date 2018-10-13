#version 400

layout(location = 0) in vec3 in_vertex;

uniform mat4 model;
uniform mat4 lightSpace;

uniform sampler2D centers;
uniform int texSize;

void main() {
	vec2 uv = vec2(gl_InstanceID % texSize, gl_InstanceID / texSize) / texSize + 1.0f / (2*texSize);
	vec3 center = texture(centers, uv).xyz;
	gl_Position = lightSpace * model * vec4(in_vertex + center, 1.0f);
}