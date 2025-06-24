#pragma once
#include "Object.hpp"

class Camera;


class Sphere : public Object {
	static int sphere_index;

protected:
	float radius;

public:
	Sphere(Transform &&transform, std::shared_ptr<Material> material, float radius = 1.0f);
	~Sphere() override;

	getset(radius)
};
