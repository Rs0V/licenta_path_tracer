#include "Cone.hpp"

Cone::Cone()
	:
	Object({}, { 1.0f, 1.0f, 1.0f, 1.0f }),
	radius(1.0f),
	height(1.0f)
{
}

Cone::Cone(Transform transform, Color color, float radius, float height)
	:
	Object(transform, color),
	radius(radius),
	height(height)
{
}

Cone::~Cone() {
}

std::tuple<std::optional<Ray>, std::optional<Ray>> Cone::HitTest(const Ray& ray) const {
	float radiusZFactor = 1.0f - (ray.location_get().z - (this->transform.location.z - this->height / 2.0f)) / height;
	float radiusAtHeight = this->radius * radiusZFactor;
	
	if (glm::distance(glm::vec2(ray.location_get()), glm::vec2(this->transform.location)) < radiusAtHeight
	and ray.location_get().z > this->transform.location.z - this->height / 2.0f
	and ray.location_get().z < this->transform.location.z + this->height / 2.0f) {
		//return this->color;
		return {};
	}
	//return Color::zero;
	return {};
}
