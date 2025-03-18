#include "PointLight.hpp"


PointLight::PointLight(Transform &&transform, Color color, float intensity, float radius)
	:
	Light(std::move(transform), color, intensity, radius)
{
}

PointLight::~PointLight() {}
