#pragma once
#include "Actor.hpp"
#include "Material.hpp"


interface Object : public Actor {
protected:
	Material* material;

	bool visible;
	bool affectWorld;

	int type;
	int index;


	Object(int type, int index);
	Object(int type, int index, Transform &&transform);

public:
	~Object() override = 0;

	getset(visible)
	getset(affectWorld)

	getter(type)
	getter(index)
};

inline Object::~Object() {}
