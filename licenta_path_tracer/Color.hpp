#pragma once
#include "glm.hpp"
#include "Utilities.hpp"

#define break4(_vec4) _vec4.x, _vec4.y, _vec4.z, _vec4.w
#define break4u(_uint) _uint & 255, _uint >> 8 & 255, _uint >> 16 & 255, _uint >> 24 & 255

// Operator returning rvalue, with same type parameter.
#define op_re(symbol) \
Color operator##symbol(const Color& other) const { \
	return { \
		this->r symbol other.r, \
		this->g symbol other.g, \
		this->b symbol other.b, \
		this->a symbol other.a, \
	}; \
}
// Operator returning rvalue, with different type parameter.
#define op_rne(symbol) \
template<class T> Color operator##symbol(T scalar) const { \
	return { \
		(float)(this->r symbol scalar), \
		(float)(this->g symbol scalar), \
		(float)(this->b symbol scalar), \
		(float)(this->a symbol scalar) \
	}; \
}


class Color : public glm::vec4 {
public:
	using glm::vec4::vec4;


	Color() = default;
	Color(const glm::vec4& vec4);
	Color(uint pixel255);

	Color(const Color&) = default;
	Color(Color&&) noexcept = default;

	Color& operator=(const Color&) = default;
	Color& operator=(Color&&) noexcept = default;

	~Color() = default;

	operator uint() const {
		return (uint)(this->r * 255)
			| ((uint)(this->g * 255) << 8)
			| ((uint)(this->b * 255) << 16)
			| ((uint)(this->a * 255) << 24)
		;
	}
	operator bool() const {
		return (this->r > 0.0f or this->g > 0.0f or this->a > 0.0f or this->a > 0.0f);
	}

	static const Color zero;
	static const Color white;
	static const Color black;
	static const Color transparent;
};
