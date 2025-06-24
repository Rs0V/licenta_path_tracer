#include "Cube.hpp"

int Cube::cube_index = 0;


Cube::Cube(Transform &&transform, std::shared_ptr<Material> material, glm::vec3 dimensions)
	:
	Object(1, Cube::cube_index++, std::move(transform), material),
	dimensions(dimensions)
{
}

Cube::~Cube() {}
