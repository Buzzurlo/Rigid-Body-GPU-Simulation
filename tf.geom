#version 400

layout(points) in;
layout(points, max_vertices = 1) out;

flat in ivec3[] voxel_ind;

uniform sampler2DArray depthTextures;

uniform ivec3 dimensions;
uniform vec3 max;
uniform vec3 min;
uniform float diam;

out vec3 out_pos;

void main() {
	vec3 curr_voxel = (voxel_ind[0] + 0.5) / dimensions;
	
	int layers = textureSize(depthTextures, 0).z;
	int i;
	int inside = 0;

	for(i = 0; i < layers; ++i) {
		float curr_depth = texture(depthTextures, vec3(curr_voxel.xy, i)).r;
		if(curr_voxel.z > curr_depth) {
			inside = 1 - inside;
		}
		else break;
	}

	out_pos = vec3(voxel_ind[0]) / dimensions * (max - min) + min + diam / 2;

	if (inside == 1)
		EmitVertex();
	EndPrimitive();
}