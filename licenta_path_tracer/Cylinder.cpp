#include "Cylinder.hpp"

int Cylinder::cylinder_index = 0;


Cylinder::Cylinder(Transform &&transform, float radius, float height)
	:
	Object(2, Cylinder::cylinder_index++, std::move(transform)),
	radius(radius),
	height(height)
{
}

Cylinder::~Cylinder() {}
