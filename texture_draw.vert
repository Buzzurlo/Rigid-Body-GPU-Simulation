#version 400

in vec2 pos;

out vec2 texel;

void main() {
	gl_Position = vec4(pos, 0.0f, 1.0f);
	texel = pos * 0.5f + 0.5f;
}