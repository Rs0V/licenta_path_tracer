#include "Light.hpp"


Light::Light(Transform &&transform, Color color, float intensity, float radius)
	:
	Actor(std::move(transform)),
	color(color),
	intensity(intensity),
	radius(radius)
{
}
