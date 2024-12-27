#pragma once
#include "Light.hpp"

class PointLight : public Light {
protected:
	float radius;

public:
	PointLight();
	PointLight(Transform transform, Color color, float intensity, float radius);
	~PointLight() override;

	getset(radius)
};
