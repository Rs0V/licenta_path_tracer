#pragma once
#include "Transform.hpp"
#include "Color.hpp"

interface Light {
protected:
	Transform transform;
	Color color;
	float intensity;

	Light(Transform transform, Color color, float intensity);

public:
	virtual ~Light() = 0;

	getterr(transform)
	getset(color)
	getset(intensity)
};

inline Light::~Light() {}
