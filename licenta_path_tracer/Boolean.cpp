#include "Boolean.hpp"


boolean::Boolean::Boolean(std::shared_ptr<Object> self, std::shared_ptr<Object> other, Type type, float blend)
	:
	Component(),
	self(self),
	other(other),
	type(type),
	blend(blend)
{
}

boolean::Boolean::~Boolean() {}
