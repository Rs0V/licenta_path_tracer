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
		Object* other;
		Type type;

	public:
		Boolean();
		Boolean(Object* other, Type type);
		~Boolean() override;

		getset(other)
		getset(type)
	};
}
