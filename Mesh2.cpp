#include "Mesh2.h"

void Mesh::AddPlane(vec3 normal, vec3 right, vec3 center, float size) {
	vec3 norm_up = normalize(cross(normal, right));
	vec3 norm_right = normalize(right);

	vec3 up_left = center + norm_up * size - norm_right * size;
	vec3 up_right = center + norm_up * size + norm_right * size;
	vec3 down_left = center - norm_up * size - norm_right * size;
	vec3 down_right = center - norm_up * size + norm_right * size;
	
	vec3 color = vec3(0.7f, 0.7f, 0.7f);
	int nVertices = vertices.size();

	Vertex v = Vertex(up_left, normal, color);
	vertices.push_back(v);
	v = Vertex(up_right, normal, color);
	vertices.push_back(v);
	v = Vertex(down_left, normal, color);
	vertices.push_back(v);
	v = Vertex(down_right, normal, color);
	vertices.push_back(v);


	indices.push_back(nVertices + 0);	// up	left
	indices.push_back(nVertices + 2);	// down left
	indices.push_back(nVertices + 1);	// up	right

	indices.push_back(nVertices + 1);	// up	right
	indices.push_back(nVertices + 2);	// down	left
	indices.push_back(nVertices + 3);	// down right
}
void Mesh::AddSphere(vec3 center, float r) {
	const int spherePrecision = 24;

	int oldNumVert = vertices.size();

	for (int i = 1; i < spherePrecision / 2; ++i) {
		double u = PI * i / (spherePrecision / 2);
		for (int j = 0; j < spherePrecision; ++j) {
			double v = 2 * PI * j / spherePrecision;

			vec3 new_point(r * sin(u) * cos(v), r * sin(u) * sin(v), r * cos(u));	// 35 ms
			vec3 color = vec3(1, 1, 1);

			Vertex vert(new_point + center, new_point, color);	// 23 ms
			vertices.push_back(vert);
		}
	}

	// front and back points
	Vertex front(
		vec3(0, 0, r) + center,		// position
		vec3(0, 0, 1),				// normal
		vec3(1, 1, 1));				// color
	Vertex back(
		vec3(0, 0, -r) + center,	// position
		vec3(0, 0, -1),				// normal
		vec3(1, 1, 1));				// color
	vertices.push_back(front);
	vertices.push_back(back);

	for (int a = 0; a < spherePrecision / 2 - 2; ++a) {
		int i;
		for (int b = 0; b < spherePrecision - 1; ++b) {
			i = a * spherePrecision + b + oldNumVert;

			indices.push_back(i);
			indices.push_back(i + spherePrecision + 1);
			indices.push_back(i + 1);

			indices.push_back(i);
			indices.push_back(i + spherePrecision);
			indices.push_back(i + spherePrecision + 1);
		}

		i = a * spherePrecision + oldNumVert;

		indices.push_back(i);
		indices.push_back(i + spherePrecision - 1);
		indices.push_back(i + spherePrecision);

		indices.push_back(i + spherePrecision);
		indices.push_back(i + spherePrecision - 1);
		indices.push_back(i + 2 * spherePrecision - 1);
	}

	// front and back cones
	int newNumVert = vertices.size();
	for (int b = 0; b < spherePrecision; ++b) {
		indices.push_back(newNumVert - 2);
		indices.push_back(oldNumVert + b);
		indices.push_back(oldNumVert + (b + 1) % spherePrecision );

		indices.push_back(newNumVert - 1);
		indices.push_back(newNumVert - 2 - spherePrecision + (b + 1) % spherePrecision);
		indices.push_back(newNumVert - 2 - spherePrecision + b);
	}
}
void Mesh::AddCube(vec3 front_top_left, vec3 dimensions) {
	vec3 color = vec3(1, 0, 0);
	vec3 front_normal = vec3(0, 0, -1);
	vec3 right_normal = vec3(1, 0, 0);
	vec3 top_normal = vec3(0, 1, 0);
	int n = vertices.size();

	Vertex vert = Vertex(front_top_left, front_normal, color);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, 0, 0);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, -dimensions.y, 0);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, -dimensions.y, 0);
	vertices.push_back(vert);

	vert.normal = right_normal;
	vert.position = front_top_left + vec3(dimensions.x, 0, 0);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, 0, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, -dimensions.y, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, -dimensions.y, 0);
	vertices.push_back(vert);

	vert.normal = top_normal;
	vert.position = front_top_left;
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, 0, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, 0, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, 0, 0);
	vertices.push_back(vert);

	vert.normal = -front_normal;
	vert.position = front_top_left + vec3(dimensions.x, 0, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, 0, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, -dimensions.y, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, -dimensions.y, dimensions.z);
	vertices.push_back(vert);

	vert.normal = -right_normal;
	vert.position = front_top_left + vec3(0, 0, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left;
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, -dimensions.y, 0);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, -dimensions.y, dimensions.z);
	vertices.push_back(vert);

	vert.normal = -top_normal;
	vert.position = front_top_left + vec3(0, -dimensions.y, 0);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, -dimensions.y, 0);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(dimensions.x, -dimensions.y, dimensions.z);
	vertices.push_back(vert);
	vert.position = front_top_left + vec3(0, -dimensions.y, dimensions.z);
	vertices.push_back(vert);

	// indices
	// front
	indices.push_back(n + 0);
	indices.push_back(n + 1);
	indices.push_back(n + 3);

	indices.push_back(n + 3);
	indices.push_back(n + 1);
	indices.push_back(n + 2);
	
	// right
	indices.push_back(n + 0 + 4);
	indices.push_back(n + 1 + 4);
	indices.push_back(n + 3 + 4);

	indices.push_back(n + 3 + 4);
	indices.push_back(n + 1 + 4);
	indices.push_back(n + 2 + 4);

	// top
	indices.push_back(n + 0 + 8);
	indices.push_back(n + 1 + 8);
	indices.push_back(n + 3 + 8);

	indices.push_back(n + 3 + 8);
	indices.push_back(n + 1 + 8);
	indices.push_back(n + 2 + 8);

	// back
	indices.push_back(n + 0 + 12);
	indices.push_back(n + 1 + 12);
	indices.push_back(n + 3 + 12);

	indices.push_back(n + 3 + 12);
	indices.push_back(n + 1 + 12);
	indices.push_back(n + 2 + 12);

	// left
	indices.push_back(n + 0 + 16);
	indices.push_back(n + 1 + 16);
	indices.push_back(n + 3 + 16);

	indices.push_back(n + 3 + 16);
	indices.push_back(n + 1 + 16);
	indices.push_back(n + 2 + 16);

	// bottom
	indices.push_back(n + 0 + 20);
	indices.push_back(n + 1 + 20);
	indices.push_back(n + 3 + 20);

	indices.push_back(n + 3 + 20);
	indices.push_back(n + 1 + 20);
	indices.push_back(n + 2 + 20);
}
bool Mesh::Load(std::string filename) {
	std::vector<vec3> temp_vert, temp_norm;
	std::vector<vec2> temp_uv;
	std::vector<Vertex> out_vert;
	std::vector<int>  out_ind;
	std::vector<std::string> unique_vert;


	FILE * obj_file = fopen(filename.c_str(), "r");
	if (obj_file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	char type[256];
	while (1) {
		fscanf(obj_file, "%s", type);

		if (strcmp(type, "v") == 0) {
			vec3 v;
			int n = fscanf(obj_file, "%f %f %f", &v.x, &v.y, &v.z);
			if(n < 3)
				printf("v values read: %d\n", n);
			temp_vert.push_back(v);
		}
		else if (strcmp(type, "vn") == 0) {
			vec3 n;
			int c = fscanf(obj_file, "%f %f %f", &n.x, &n.y, &n.z);
			if(c < 3)
				printf("vn values read: %d\n", c);
			temp_norm.push_back(normalize(n));
		}
		else if (strcmp(type, "vt") == 0) {
			vec2 uv;
			int n = fscanf(obj_file, "%f %f", &uv.x, &uv.y);
			if(n < 2)
				printf("vt values read: %d\n", n);
			temp_uv.push_back(uv);
		}
		else if (strcmp(type, "f") == 0) {
			int vi, uvi, ni;
			char vert_string[24];
			for (int t = 0; t < 3; ++t) {
				fscanf(obj_file, "%s", vert_string);
				int i, n = unique_vert.size();
				for (i = 0; i < n; ++i) {
					if (strcmp(unique_vert[i].c_str(), vert_string) == 0) {
						out_ind.push_back(i);
						break;
					}
				}
				if (i < n)
					continue;

				out_ind.push_back(n);
				unique_vert.push_back(vert_string);
				sscanf(vert_string, "%d/%d/%d", &vi, &uvi, &ni);
				vi = vi < 0 ? temp_vert.size() - vi : vi - 1;
				uvi = uvi < 0 ? temp_uv.size() - uvi : uvi - 1;
				ni = ni < 0 ? temp_norm.size() - ni : ni - 1;
				Vertex vert(temp_vert[vi], -temp_norm[ni], vec3(temp_uv[uvi], 0.0f));
				out_vert.push_back(vert);

				if (n != out_vert.size() - 1)
					printf("error, sizes dont match! %d %d\n", n, out_vert.size());
			}

		}

		if (feof(obj_file))
			break;
	}

	vertices = out_vert;
	indices = out_ind;

	fclose(obj_file);
}