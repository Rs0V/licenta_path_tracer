#pragma once
#include "Object.hpp"


class Cylinder : public Object {
	static int cylinder_index;

protected:
	float radius, height;

public:
	Cylinder(Transform &&transform, const Material* material, float radius = 1.0f, float height = 2.0f);
	~Cylinder() override;

	getter(radius)
	getter(height)
};
