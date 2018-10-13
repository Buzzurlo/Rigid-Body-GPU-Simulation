#version 400

layout (location = 0) out float out_color;

void main() {
	//out_color = vec3(gl_FragCoord.z, 1 - gl_FragCoord.z, 0.0f);
	out_color = gl_FragCoord.z;
}