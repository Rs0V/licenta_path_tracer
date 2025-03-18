#include "Sphere.hpp"

int Sphere::sphere_index = 0;


Sphere::Sphere(Transform &&transform, float radius)
	:
	Object(0, Sphere::sphere_index++, std::move(transform)),
	radius(radius)
{
}

Sphere::~Sphere() {}
