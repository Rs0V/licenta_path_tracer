#pragma once
#include "Component.hpp"
#include "Object.hpp"


namespace boolean {
	enum Type {
		Union,
		Intersect,
		Difference
	};

	class Boolean : public Component {
	protected:
		Object *const self;
		Object* other;

		Type type;
		float blend;

	public:
		Boolean(Object *self, Object *other, Type type, float blend = 0.5f);
		~Boolean() override;

		getter(self)
		getset(other)

		getset(type)
		getset(blend)
	};
}
