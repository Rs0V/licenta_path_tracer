#pragma once
#include "glm.hpp"
#include "Utilities.hpp"
#include "Color.hpp"

class Ray {
protected:
	glm::vec3 location, direction;
	Color color;

public:
	Ray();
	Ray(glm::vec3 location, glm::vec3 direction, Color color);
	~Ray();

	Ray& operator=(const Ray& other) {
		this->location = other.location;
		this->direction = other.direction;
		this->color = other.color;
		return *this;
	}

	getset(location)
	getset(direction)
	getset(color)
};
