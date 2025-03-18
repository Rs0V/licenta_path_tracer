#pragma once
#include "glm.hpp"


class Transform {
public:
	glm::vec3 location, rotation, scale;


	Transform();
	Transform(glm::vec3 location, glm::vec3 rotation, glm::vec3 scale);

	Transform(const Transform&) = default;
	Transform(Transform&&) noexcept = default;

	Transform& operator=(const Transform&) = default;
	Transform& operator=(Transform&&) noexcept = default;

	~Transform() = default;
};
