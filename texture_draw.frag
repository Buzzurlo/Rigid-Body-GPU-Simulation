#version 400

in vec2 texel;

uniform sampler2D depthTextures;

out vec4 out_color;

float LinearizeDepth(in vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 200.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture2D(depthTextures, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main() {
	float depth = LinearizeDepth(texel);
	out_color = vec4(vec3(depth), 1.0f);
}