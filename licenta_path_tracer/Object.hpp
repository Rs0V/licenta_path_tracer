#pragma once
#include "glm.hpp"
#include "Transform.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "vector"
#include "optional"
#include "tuple"

interface Object {
protected:
	Transform transform;
	Color color;

	std::vector<Component*> components;
	bool visible;
	bool affectWorld;

	Object();
	Object(Transform transform, Color color);

public:
	virtual ~Object() = 0;
	virtual std::tuple<std::optional<Ray>, std::optional<Ray>> HitTest(const Ray& ray) const = 0;

	getterr(transform)
	getset(color)
	getterr(components)
	getset(visible)
	getset(affectWorld)
};

inline Object::~Object() {}
