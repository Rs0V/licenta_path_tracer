// RAYMARCHING OBJECT STRUCTS
#pragma once
#include "glm.hpp"

namespace rmo {
	struct Sphere {
		glm::vec3 location;
		float radius;
	};

	struct Cube {
		glm::vec3 location;
		float lenX;
		float lenY;
		float lenZ;
	};

	struct Cylinder {
		glm::vec3 location;
		float radius;
		float height;
	};

	struct Cone {
		glm::vec3 location;
		float radius;
		float height;
	};
}
