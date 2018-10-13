#pragma once

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include "glm\gtx\transform.hpp"

using namespace glm;

class Camera {
public:
	Camera();

	void setPosition(vec3 pos);
	void setPosition(float x, float y, float z);
	void moveAxis(float speed, vec3 dir);
	void moveRelative(float speed, vec3 dir);
	void moveParallel(float speed, float angle);
	void point(vec3 dir);
	void fromEulerAngles(float x_angle, float y_angle, float z_angle);

	mat4 getViewMatrix();

private:
	vec3 position;
	quat orientation;
};