#include "Parent.hpp"


Parent::Parent(Object *self, const Object *parent)
	:
	self(self),
	parent(parent),
	last_parent_transform(parent->transform_getrc())
{
}

Parent::~Parent() {}



void Parent::applyTransform() {
	glm::vec3 deltaLoc   = parent->transform_getrc().location - last_parent_transform.location;
	glm::vec3 deltaRot   = parent->transform_getrc().rotation - last_parent_transform.rotation;
	glm::vec3 deltaScale = parent->transform_getrc().scale    / last_parent_transform.scale;

	self->translate(deltaLoc);
	self->rotate(deltaRot, 1, deltaLoc);
	self->scale(deltaScale, 1, deltaLoc);

	this->applyOffset();
}

void Parent::applyOffset() {
	last_parent_transform.location = parent->transform_getrc().location;
	last_parent_transform.rotation = parent->transform_getrc().rotation;
	last_parent_transform.scale    = parent->transform_getrc().scale;
}
