#include "Object.hpp"

Object::Object()
	:
	transform(Transform()),
	color(color),
	components(),
	visible(true),
	affectWorld(true)
{}

Object::Object(Transform transform, Color color)
	:
	transform(transform),
	color(color),
	components(),
	visible(true),
	affectWorld(true)
{}
