#include "Object.hpp"


Object::Object(int type, int index)
	:
	Actor(),
	material(nullptr),
	visible(true),
	affectWorld(true),
	type(type),
	index(index)
{
}

Object::Object(int type, int index, Transform &&transform)
	:
	Actor(std::move(transform)),
	material(nullptr),
	visible(true),
	affectWorld(true),
	type(type),
	index(index)
{
}
