#include "Camera.hpp"


Camera::Camera(Transform &&transform)
	:
	Actor(std::move(transform))
{
}
