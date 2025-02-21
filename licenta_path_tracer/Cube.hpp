#pragma once
#include "Object.hpp"

class Cube : public Object {
	static int cube_index;

protected:
	glm::vec3 dimensions;

public:
	Cube();
	Cube(Transform transform, Color color, glm::vec3 dimensions);
	~Cube() override;

	std::tuple<std::optional<Ray>, std::optional<Ray>> HitTest(const Ray& ray) const override;

	getter(dimensions)
};
