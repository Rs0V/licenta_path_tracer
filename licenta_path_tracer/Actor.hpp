#pragma once
#include "Transform.hpp"
#include "Component.hpp"
#include "vector"


interface Actor {
protected:
	Transform transform;
	glm::vec3 right, forward, up;
	std::vector<Component*> components;


	Actor();
	Actor(Transform &&transform);

public:
	virtual ~Actor() = 0;

	getterr(transform)

	getterr(right)
	getterr(forward)
	getterr(up)

	getterr(components)
};

inline Actor::~Actor() {}
