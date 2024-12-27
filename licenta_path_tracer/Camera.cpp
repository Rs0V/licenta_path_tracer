#include "Camera.hpp"

Camera::Camera()
	:
	transform({}),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 1.0f, 0.0f),
	up(0.0f, 0.0f, 1.0f)
{}

Camera::Camera(Transform transform)
	:
	transform(transform),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 1.0f, 0.0f),
	up(0.0f, 0.0f, 1.0f)
{}

Camera::~Camera() {}
