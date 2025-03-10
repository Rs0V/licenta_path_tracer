#include "Parent.hpp"

Parent::Parent(Object* self, const Object* parent)
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

	self->transform_getr().location += deltaLoc;
	self->transform_getr().rotation += deltaRot;
	self->transform_getr().scale    *= deltaScale;

	glm::mat4 dpMat = glm::translate(glm::mat4(1.0f), -parent->transform_getrc().location);
	glm::mat4 idpMat = glm::translate(glm::mat4(1.0f), parent->transform_getrc().location);

	glm::mat4 rotMat = glm::mat4(1.0f)
		* glm::rotate(glm::mat4(1.0f), -deltaRot.x, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), -deltaRot.y, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), -deltaRot.z, glm::vec3(0.0f, 0.0f, 1.0f))
	;
	glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), deltaScale);

	self->transform_getr().location = glm::vec3(idpMat * rotMat * scaleMat * dpMat * glm::vec4(self->transform_getr().location, 1.0f));

	this->applyOffset();
}

void Parent::applyOffset() {
	last_parent_transform.location = parent->transform_getrc().location;
	last_parent_transform.rotation = parent->transform_getrc().rotation;
	last_parent_transform.scale    = parent->transform_getrc().scale;
}
