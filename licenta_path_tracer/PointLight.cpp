#include "PointLight.hpp"

PointLight::PointLight()
	:
	Light({}, { 1.0f, 1.0f, 1.0f, 1.0f }, 1.0f),
	radius(10.0f)
{}

PointLight::PointLight(Transform transform, Color color, float intensity, float radius)
	:
	Light(transform, color, intensity),
	radius(radius)
{}

PointLight::~PointLight() {}
