#pragma once
#include "Object.hpp"


class Cone : public Object {
	static int cone_index;

protected:
	float radius, height;

public:
	Cone(Transform &&transform, float radius = 1.0f, float height = 2.0f);
	~Cone() override;

	getter(radius)
	getter(height)
};
