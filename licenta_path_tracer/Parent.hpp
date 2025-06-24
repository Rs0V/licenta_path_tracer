#pragma once
#include "Component.hpp"
#include "Object.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/transform.hpp"
#include "gtc/type_ptr.hpp"


class Parent : public Component {
protected:
	std::shared_ptr<Object> self;
	std::shared_ptr<Object> parent;
	Transform last_parent_transform;

public:
	Parent(std::shared_ptr<Object> self, std::shared_ptr<Object> parent);
	~Parent() override;

	void applyTransform();
	void applyOffset();
};
