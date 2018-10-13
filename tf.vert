#version 400

out ivec3 voxel_ind;

uniform ivec3 dimensions;
uniform vec3 max;
uniform vec3 min;
uniform float diam;

void main() {
	int i  = gl_VertexID;
	int i_x = i % dimensions.x;
	int i_y = i / dimensions.x % dimensions.y;
	int i_z = i / (dimensions.x * dimensions.y);
	
	/*
	float x = float(i_x) / dimensions.x * (max.x - min.x) + min.x + diam / 2;
	float y = float(i_y) / dimensions.y * (max.y - min.y) + min.y + diam / 2;
	float z = float(i_z) / dimensions.z * (max.z - min.z) + min.z + diam / 2;
	*/
	
	
	float x = (float(i_x) + 0.5) / dimensions.x;
	float y = (float(i_y) + 0.5) / dimensions.y;
	float z = (float(i_z) + 0.5) / dimensions.z;
	
	voxel_ind = ivec3(i_x, i_y, i_z);
}