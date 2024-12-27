#pragma once
#include "Transform.hpp"
#include "Utilities.hpp"

class Camera {
protected:
	Transform transform;
	glm::vec3 right, forward, up;

public:
	Camera();
	Camera(Transform transform);
	~Camera();

	getterr(transform)
	getterr(right)
	getterr(forward)
	getterr(up)
};
