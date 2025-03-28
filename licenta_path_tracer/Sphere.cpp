#include "Sphere.hpp"
#include "Camera.hpp"

int Sphere::sphere_index = 0;


Sphere::Sphere(Transform &&transform, const Material* material, float radius)
	:
	Object(0, Sphere::sphere_index++, std::move(transform), material),
	radius(radius)
{
}

Sphere::~Sphere() {}
