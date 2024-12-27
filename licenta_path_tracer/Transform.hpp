#pragma once
#include "Component.hpp"
#include "glm.hpp"

class Transform : public Component {
public:
	glm::vec3 location, rotation, scale;

	Transform();
	Transform(glm::vec3 location, glm::vec3 rotation, glm::vec3 scale);
	~Transform() override;
};
