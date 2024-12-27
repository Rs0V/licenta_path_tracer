#include "Cylinder.hpp"

Cylinder::Cylinder()
	:
	Object({}, { 1.0f, 1.0f, 1.0f, 1.0f }),
	radius(1.0f),
	height(1.0f)
{
}

Cylinder::Cylinder(Transform transform, Color color, float radius, float height)
	:
	Object(transform, color),
	radius(radius),
	height(height)
{
}

Cylinder::~Cylinder() {
}

std::tuple<std::optional<Ray>, std::optional<Ray>> Cylinder::HitTest(const Ray& ray) const {
	if (glm::distance(glm::vec2(ray.location_get()), glm::vec2(this->transform.location)) < this->radius
	and ray.location_get().z > this->transform.location.z - this->height / 2.0f
	and ray.location_get().z < this->transform.location.z + this->height / 2.0f) {
		//return this->color;
		return {};
	}
	//return Color::zero;
	return {};
}
