#include "Transform.hpp"

Transform::Transform()
	:
	location(0.0f, 0.0f, 0.0f),
	rotation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f)
{}

Transform::Transform(glm::vec3 location, glm::vec3 rotation, glm::vec3 scale)
	:
	location(location),
	rotation(rotation),
	scale(scale)
{}

Transform::~Transform() {}
