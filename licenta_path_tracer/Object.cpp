#include "Object.hpp"


Object::Object(int type, int index, const Material *material)
	:
	Actor(),
	material(material),
	visible(true),
	affectWorld(true),
	type(type),
	index(index)
{
}

Object::Object(int type, int index, Transform &&transform, const Material *material)
	:
	Actor(std::move(transform)),
	material(material),
	visible(true),
	affectWorld(true),
	type(type),
	index(index)
{
}
