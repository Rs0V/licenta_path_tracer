#pragma once
#include "Object.hpp"


class Sphere : public Object {
	static int sphere_index;

protected:
	float radius;

public:
	Sphere(Transform &&transform, const Material *material, float radius = 1.0f);
	~Sphere() override;

	getset(radius)
};
