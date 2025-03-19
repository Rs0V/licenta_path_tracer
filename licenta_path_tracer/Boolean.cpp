#include "Boolean.hpp"


boolean::Boolean::Boolean(Object *self, Object *other, Type type, float blend)
	:
	Component(),
	self(self),
	other(other),
	type(type),
	blend(blend)
{
}

boolean::Boolean::~Boolean() {}
