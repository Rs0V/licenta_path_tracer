#pragma once
#include "Actor.hpp"
#include "Material.hpp"

class Camera;


interface Object : public Actor {
protected:
	const Material* material;

	bool visible;
	bool affectWorld;

	int type;
	int index;


	Object(int type, int index, const Material *material);
	Object(int type, int index, Transform &&transform, const Material* material);

public:
	~Object() override = 0;

	getter(material)

	getset(visible)
	getset(affectWorld)

	getter(type)
	getter(index)
};

inline Object::~Object() {}
