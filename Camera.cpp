#include "Camera.h"

#include <stdio.h>

Camera::Camera()
{
	position = vec3(0);
	orientation = angleAxis(0.0f, vec3(0, 0, -1));
}

void Camera::setPosition(vec3 pos)
{
	position = pos;
}

void Camera::setPosition(float x, float y, float z)
{
	position = vec3(x, y, z);
}

void Camera::moveAxis(float speed, vec3 dir)
{
	dir = normalize(dir);
	position += speed * dir;
}

void Camera::moveRelative(float speed, vec3 dir)
{
	dir = normalize(dir);
	position += rotate(orientation, speed * dir);
}

void Camera::moveParallel(float speed, float angle)
{
	quat rot = angleAxis(angle, vec3(0, 1, 0));
	position += rotate(rot, speed * vec3(0, 0, -1));
}

void Camera::point(vec3 pos)
{
	vec3 dir = normalize(pos - position);
	orientation = rotation(vec3(0, 0, -1), dir);
}

void Camera::fromEulerAngles(float y_angle, float x_angle, float z_angle)
{
	orientation = quat(vec3(x_angle, y_angle, z_angle));
}

mat4 Camera::getViewMatrix()
{
	vec3 pos = position;
	vec3 dir = rotate(orientation, vec3(0, 0, -1));
	vec3 up = rotate(orientation, vec3(0, 1, 0));
	return lookAt(pos, pos + dir, up);
}
