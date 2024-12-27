#pragma once
#include "Object.hpp"

class Cylinder : public Object {
protected:
	float radius, height;

public:
	Cylinder();
	Cylinder(Transform transform, Color color, float radius, float height);
	~Cylinder() override;

	std::tuple<std::optional<Ray>, std::optional<Ray>> HitTest(const Ray& ray) const override;

	getter(radius)
	getter(height)
};
