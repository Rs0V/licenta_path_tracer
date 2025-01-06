// RAYMARCHING OBJECT STRUCTS
#pragma once
#include "glm.hpp"
#include "Sphere.hpp"
#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Cone.hpp"


namespace rmo {
	struct Sphere {
		glm::vec3 location;
		float radius;

		Sphere& operator=(const ::Sphere &sphere) {
			this->location = sphere.transform_getcr().location;
			this->radius = sphere.radius_get();
			return *this;
		}
	};

	struct Cube {
		glm::vec3 location;
		privb(float pad0)
		glm::vec3 dimensions;
		privb(float pad1)

		Cube& operator=(const ::Cube& cube) {
			this->location = cube.transform_getcr().location;
			this->dimensions = cube.dimensions_get();
			return *this;
		}
	};

	struct Cylinder {
		glm::vec3 location;
		float radius;
		float height;
		privb(float pad0)
		privb(float pad1)
		privb(float pad2)

		Cylinder& operator=(const ::Cylinder& cylinder) {
			this->location = cylinder.transform_getcr().location;
			this->radius = cylinder.radius_get();
			this->height = cylinder.height_get();
			return *this;
		}
	};

	struct Cone {
		glm::vec3 location;
		float radius;
		float height;
		privb(float pad0)
		privb(float pad1)
		privb(float pad2)

		Cone& operator=(const ::Cone& cone) {
			this->location = cone.transform_getcr().location;
			this->radius = cone.radius_get();
			this->height = cone.height_get();
			return *this;
		}
	};
}
