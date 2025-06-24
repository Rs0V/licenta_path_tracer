#include "Object.hpp"


Object::Object(int type, int index, std::shared_ptr<Material> material)
	:
	Actor(),
	material(material),
	visible(true),
	affectWorld(true),
	type(type),
	index(index)
{
}

Object::Object(int type, int index, Transform &&transform, std::shared_ptr<Material> material)
	:
	Actor(std::move(transform)),
	material(material),
	visible(true),
	affectWorld(true),
	type(type),
	index(index)
{
}
