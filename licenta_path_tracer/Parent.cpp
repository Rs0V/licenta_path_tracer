#include "Parent.hpp"

Parent::Parent(Object* self, const Object* parent)
	:
	self(self),
	parent(parent)
{
}

Parent::~Parent() {}

void Parent::applyTransform() {

}

void Parent::applyOffset() {

}
