#include "Cube.hpp"

int Cube::cube_index = 0;


Cube::Cube(Transform &&transform, const Material* material, glm::vec3 dimensions)
	:
	Object(1, Cube::cube_index++, std::move(transform), material),
	dimensions(dimensions)
{
}

Cube::~Cube() {}
