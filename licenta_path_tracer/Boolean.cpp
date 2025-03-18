#include "Boolean.hpp"


boolean::Boolean::Boolean(Object *self, Object *other, Type type)
	:
	self(self),
	other(other),
	type(type)
{
}

boolean::Boolean::~Boolean() {}
