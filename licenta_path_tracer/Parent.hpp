#pragma once
#include "Component.hpp"
#include "Object.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/transform.hpp"
#include "gtc/type_ptr.hpp"


class Parent : public Component {
protected:
	Object *self;
	const Object *parent;
	Transform last_parent_transform;

public:
	Parent(Object* self, const Object *parent);
	~Parent() override;

	void applyTransform();
	void applyOffset();
};
