#pragma once
#include "Component.hpp"
#include "Object.hpp"


class Parent : public Component {
protected:
	Object *self;
	const Object *parent;

public:
	Parent(Object* self, const Object *parent);
	~Parent() override;

	void applyTransform();
	void applyOffset();
};
