#include "Sphere.hpp"

int Sphere::sphere_index = 0;

Sphere::Sphere()
	:
	Object({}, { 1.0f, 1.0f, 1.0f, 1.0f }),
	radius(1.0f)
{
	this->type = 0;
	this->index = Sphere::sphere_index++;
}

Sphere::Sphere(Transform transform, Color color, float radius)
	:
	Object(transform, color),
	radius(radius)
{
	this->type = 0;
	this->index = Sphere::sphere_index++;
}

Sphere::~Sphere() {}

std::tuple<std::optional<Ray>, std::optional<Ray>> Sphere::HitTest(const Ray& ray) const {
	float t = glm::dot(this->transform.location - ray.location_get(), ray.direction_get());
	glm::vec3 p = ray.location_get() + ray.direction_get() * t;

	float y = glm::distance(this->transform.location, p);
	if (y > this->radius) {
		return { none, none };
	}

	float x = glm::sqrt(this->radius * this->radius - y * y);
	float t1 = t - x;
	float t2 = t + x;

	glm::vec3 p1 = ray.location_get() + ray.direction_get() * t1;
	glm::vec3 p2 = ray.location_get() + ray.direction_get() * t2;

	Ray ray1 = Ray(p1, glm::reflect(ray.direction_get(), glm::normalize(p1 - this->transform.location)), ray.color_get() * this->color);
	Ray ray2 = Ray(p2, glm::reflect(ray.direction_get(), -glm::normalize(p2 - this->transform.location)), ray.color_get() * this->color);

	return { ray1, ray2 };
}
