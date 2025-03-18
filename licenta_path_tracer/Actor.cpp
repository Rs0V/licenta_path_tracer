#include "Actor.hpp"


Actor::Actor()
	:
	transform(Transform()),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 1.0f, 0.0f),
	up(0.0f, 0.0f, 1.0f),
	components()
{
}

Actor::Actor(Transform &&transform)
	:
	transform(std::move(transform)),
	right(1.0f, 0.0f, 0.0f),
	forward(0.0f, 1.0f, 0.0f),
	up(0.0f, 0.0f, 1.0f),
	components()
{
}
