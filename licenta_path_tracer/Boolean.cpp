#include "Boolean.hpp"

boolean::Boolean::Boolean()
	:
	other(nullptr),
	type(Type::Union)
{}

boolean::Boolean::Boolean(Object* other, Type type)
	:
	other(other),
	type(type)
{}

boolean::Boolean::~Boolean(){}
