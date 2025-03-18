#pragma once
#include "Actor.hpp"


class Camera : public Actor {
public:
	Camera() = default;
	Camera(Transform &&transform);
};
