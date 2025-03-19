#include "Cone.hpp"

int Cone::cone_index = 0;


Cone::Cone(Transform &&transform, const Material* material, float radius, float height)
	:
	Object(3, Cone::cone_index++, std::move(transform), material),
	radius(radius),
	height(height)
{
}

Cone::~Cone() {}
