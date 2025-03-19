#pragma once
#include "Utilities.hpp"
#include "Component.hpp"


interface Material {
protected:
	int type;
	int index;

	Material(int type, int index) : type(type), index(index) {}

public:
	virtual ~Material() = 0;

	getter(type)
	getter(index)
};

inline Material::~Material() {}
