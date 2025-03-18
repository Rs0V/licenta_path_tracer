#pragma once
#include "Actor.hpp"
#include "Color.hpp"


interface Light : public Actor {
protected:
	Color color;
	float intensity;
	float radius;


	Light(Transform &&transform, Color color = Color::white, float intensity = 2.0f, float radius = 25.0f);

public:
	virtual ~Light() = 0;

	getset(color)
	getset(intensity)
	getset(radius)
};

inline Light::~Light() {}
