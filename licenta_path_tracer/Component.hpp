#pragma once
#include "Utilities.hpp"

interface Component {
public:
	virtual ~Component() = 0;
};

inline Component::~Component() {}
