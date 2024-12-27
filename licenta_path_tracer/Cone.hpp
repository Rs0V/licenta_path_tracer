#pragma once
#include "Object.hpp"

class Cone : public Object {
protected:
	float radius, height;

public:
	Cone();
	Cone(Transform transform, Color color, float radius, float height);
	~Cone() override;

	std::tuple<std::optional<Ray>, std::optional<Ray>> HitTest(const Ray& ray) const override;

	getter(radius)
	getter(height)
};
