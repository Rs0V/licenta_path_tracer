#pragma once
#include "Component.hpp"
#include "Object.hpp"


namespace boolean {
	enum class Type {
		Intersect,
		Union,
		Difference
	};

	class Boolean : public Component {
	protected:
		Object *const self;
		Object* other;
		Type type;

	public:
		Boolean(Object *self, Object *other, Type type);
		~Boolean() override;

		getter(self)
		getset(other)
		getset(type)
	};
}
