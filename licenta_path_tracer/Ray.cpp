#include "Ray.hpp"

Ray::Ray()
	:
	location(0.0f, 0.0f, 0.0f),
	direction(0.0f, 1.0f, 0.0f),
	color(0.0f, 0.0f, 0.0f, 1.0f)
{}

Ray::Ray(glm::vec3 location, glm::vec3 direction, Color color)
	:
	location(location),
	direction(direction),
	color(color)
{}

Ray::~Ray() {}
