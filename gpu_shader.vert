#version 400

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpace;

uniform sampler2D centers;
uniform sampler2D vel;
uniform sampler2D shadowMap;
uniform int texSize;

out vec3 fragColor;
out vec3 fragNorm;
out vec3 fragPos;
out vec4 fragLightPos;

void main() {
	vec2 uv = vec2(gl_InstanceID % texSize, gl_InstanceID / texSize) / texSize + 1.0f / (2*texSize);
	vec3 center = texture(centers, uv).xyz;
	if(gl_InstanceID == 0)
		center = vec3(0.0f, 0.0f, 0.0f);

	vec3 world_pos = vec3(model * vec4(position + center, 1.0f));
	mat4 VP = projection * view;
	gl_Position = VP * vec4(world_pos, 1.0f);
	fragNorm = mat3(transpose(inverse(model))) * normalize(normal);
	fragColor = color * vec3(sin(gl_InstanceID) / 2 + 0.5f, cos(gl_InstanceID) / 2 + 0.5f, gl_InstanceID / 256.0f) * 0.5;
	//fragColor = texture(vel, uv).xyz;
	if(gl_InstanceID == 0)
		fragColor = vec3(0.0f, 0.0f, 0.0f);
	//fragColor = world_pos;
	fragPos = world_pos;
	fragLightPos = lightSpace * model * vec4(position + center, 1.0f);
}