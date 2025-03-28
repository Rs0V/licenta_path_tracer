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

	/// <param name="type">- 0 = world|absolute;	1 = world|delta;	2 = local|delta</param>
	void translate(glm::vec3 value, int type = 1);
	/// <param name="type">- 0 = world|absolute;	1 = world|delta;	2 = local|delta</param>
	void rotate(glm::vec3 value, int type = 1, std::optional<glm::vec3> pivot = none);
	/// <param name="type">- 0 = world|absolute;	1 = world|delta;	2 = local|delta</param>
	void scale(glm::vec3 value, int type = 1, std::optional<glm::vec3> pivot = none);

	getterr(transform)

	getterr(right)
	getterr(forward)
	getterr(up)

	getterr(components)
};

inline Actor::~Actor() {}
