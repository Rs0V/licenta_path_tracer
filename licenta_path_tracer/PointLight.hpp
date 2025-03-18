#pragma once
#include "Light.hpp"


class PointLight : public Light {
public:
	PointLight(Transform &&transform, Color color = Color::white, float intensity = 2.0f, float radius = 25.0f);
	~PointLight() override;
};
