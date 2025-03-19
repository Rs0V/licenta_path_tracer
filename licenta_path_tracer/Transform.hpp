#pragma once
#include "glm.hpp"


class Transform {
public:
	glm::vec3 location, rotation, scale;


	Transform(
		glm::vec3 location = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 scale    = glm::vec3(1.0f, 1.0f, 1.0f)
	);

	Transform(const Transform&) = default;
	Transform(Transform&&) noexcept = default;

	Transform& operator=(const Transform&) = default;
	Transform& operator=(Transform&&) noexcept = default;

	~Transform() = default;
};
