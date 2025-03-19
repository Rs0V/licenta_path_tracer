#pragma once
#include "Utilities.hpp"


interface Component {
protected:
	Component() = default;

public:
	virtual ~Component() = 0;
};

inline Component::~Component() {}
