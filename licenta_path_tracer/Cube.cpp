#include "Cube.hpp"

int Cube::cube_index = 0;

Cube::Cube()
	:
	Object({}, { 1.0f, 1.0f, 1.0f, 1.0f }),
	dimensions(1.0f)
{
	this->type = 1;
	this->index = Cube::cube_index++;
}

Cube::Cube(Transform transform, Color color, glm::vec3 dimensions)
	:
	Object(transform, color),
	dimensions(dimensions)
{
	this->type = 1;
	this->index = Cube::cube_index++;
}

Cube::~Cube() {
}

std::tuple<std::optional<Ray>, std::optional<Ray>> Cube::HitTest(const Ray& ray) const {
	/*
	if (ray.location_get().x > this->transform.location.x - this->length / 2.0f
	and ray.location_get().x < this->transform.location.x + this->length / 2.0f
	and ray.location_get().y > this->transform.location.y - this->length / 2.0f
	and ray.location_get().y < this->transform.location.y + this->length / 2.0f
	and ray.location_get().z > this->transform.location.z - this->length / 2.0f
	and ray.location_get().z < this->transform.location.z + this->length / 2.0f) {
		//return this->color;
		return {};
	}
	//return Color::zero;
	*/
	return {};
}
