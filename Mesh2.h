#pragma once

#include <string>
#include <vector>
#include <GL\glew.h>
#include "glm\glm.hpp"

using namespace glm;

const double PI = 3.14159;

class Vertex {
public:
	vec3 position;
	vec3 normal;
	vec3 color;

	Vertex() {}
	Vertex(vec3 pos) :
		position(pos),
		normal(vec3(0,1,0)),
		color(vec3(1,0,0)) {}
	Vertex(vec3 pos, vec3 norm) :
		position(pos),
		normal(norm),
		color(vec3(1, 0, 0)) {}
	Vertex(vec3 pos, vec3 norm, vec3 col) :
		position(pos),
		normal(norm),
		color(col) {}
	void AddVertexFormat(int v_attr, int n_attr, int c_attr) {
		glVertexAttribPointer(v_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glVertexAttribPointer(n_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));
		glVertexAttribPointer(c_attr, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3) * 2));
	}
};

class VertexUV {
public:
	vec3 position;
	vec3 normal;
	vec2 uv;

	VertexUV() {}
	VertexUV(vec3 pos) :
		position(pos),
		normal(vec3(0, 1, 0)),
		uv(vec2(0, 0)) {}
	VertexUV(vec3 pos, vec3 norm) :
		position(pos),
		normal(norm),
		uv(vec2(0, 0)) {}
	VertexUV(vec3 pos, vec3 norm, vec2 _uv) :
		position(pos),
		normal(norm),
		uv(_uv) {}

	void AddVertexFormat(int v_attr, int n_attr, int uv_attr) {
		glVertexAttribPointer(v_attr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexUV), (void*)0);
		glVertexAttribPointer(n_attr, 3, GL_FLOAT, GL_FALSE, sizeof(VertexUV), (void*)sizeof(vec3));
		glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE, sizeof(VertexUV), (void*)(sizeof(vec3) * 2));
	}
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<int>	indices;

	unsigned int GetVertSize() { return vertices.size() * sizeof(Vertex); }
	unsigned int GetIndSize() { return indices.size() * sizeof(int); }
	void CalcAABB(vec3& _min, vec3& _max) {
		_min = vec3(INFINITY, INFINITY, INFINITY);
		_max = vec3(-INFINITY, -INFINITY, -INFINITY);
		for (int i = 0; i < vertices.size(); ++i) {
			_min.x = min(_min.x, vertices[i].position.x);
			_min.y = min(_min.y, vertices[i].position.y);
			_min.z = min(_min.z, vertices[i].position.z);

			_max.x = max(_max.x, vertices[i].position.x);
			_max.y = max(_max.y, vertices[i].position.y);
			_max.z = max(_max.z, vertices[i].position.z);
		}
	}

	void AddSphere(vec3 center, float r);
	void AddPlane(vec3 normal, vec3 right, vec3 center, float size);

	// TODO
	bool Load(std::string filename);
	void LoadWithTexture(std::string filename) {}
	void AddCube(vec3 front_top_left, vec3 dimensions);
};