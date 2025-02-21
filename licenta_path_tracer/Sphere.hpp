#pragma once
#include "Object.hpp"

class Sphere : public Object {
	static int sphere_index;

protected:
	float radius;

public:
	Sphere();
	Sphere(Transform transform, Color color, float radius);
	~Sphere() override;

	std::tuple<std::optional<Ray>, std::optional<Ray>> HitTest(const Ray& ray) const override;

	getset(radius)
};
