#include "Color.hpp"

const Color Color::zero        = Color(0.0f, 0.0f, 0.0f, 0.0f);
const Color Color::white       = Color(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::black       = Color(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::transparent = Color(1.0f, 1.0f, 1.0f, 0.0f);


Color::Color(const glm::vec3& vec3)
	:
	glm::vec4(vec3, 1.0f)
{
	this->r = glm::clamp(this->r, 0.0f, 1.0f);
	this->g = glm::clamp(this->g, 0.0f, 1.0f);
	this->b = glm::clamp(this->b, 0.0f, 1.0f);
	this->a = glm::clamp(this->a, 0.0f, 1.0f);
}

Color::Color(const glm::vec4& vec4)
	:
	glm::vec4(vec4)
{
	this->r = glm::clamp(this->r, 0.0f, 1.0f);
	this->g = glm::clamp(this->g, 0.0f, 1.0f);
	this->b = glm::clamp(this->b, 0.0f, 1.0f);
	this->a = glm::clamp(this->a, 0.0f, 1.0f);
}

Color::Color(uint pixel255)
	:
	glm::vec4(
		pixel255 & 255,
		pixel255 >> 8 & 255,
		pixel255 >> 16 & 255,
		pixel255 >> 24 & 255
	)
{
}
