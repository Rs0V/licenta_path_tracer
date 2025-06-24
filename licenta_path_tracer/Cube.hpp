#pragma once
#include "Object.hpp"


class Cube : public Object {
	static int cube_index;

protected:
	glm::vec3 dimensions;

public:
	Cube(Transform &&transform, std::shared_ptr<Material> material, glm::vec3 dimensions = glm::vec3(1.0f, 1.0f, 1.0f));
	~Cube() override;

	getter(dimensions)
};
