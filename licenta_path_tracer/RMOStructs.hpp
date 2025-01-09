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
		glm::vec4 color;

		Sphere& operator=(const ::Sphere &sphere) {
			this->location = sphere.transform_getcr().location;
			this->radius = sphere.radius_get();
			this->color = sphere.color_get();
			return *this;
		}
	};

	struct Cube {
		glm::vec3 location;
		privb(float pad0)
		glm::vec3 dimensions;
		privb(float pad1)
		glm::vec4 color;

		Cube& operator=(const ::Cube& cube) {
			this->location = cube.transform_getcr().location;
			this->dimensions = cube.dimensions_get();
			this->color = cube.color_get();
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
		glm::vec4 color;

		Cylinder& operator=(const ::Cylinder& cylinder) {
			this->location = cylinder.transform_getcr().location;
			this->radius = cylinder.radius_get();
			this->height = cylinder.height_get();
			this->color = cylinder.color_get();
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
		glm::vec4 color;

		Cone& operator=(const ::Cone& cone) {
			this->location = cone.transform_getcr().location;
			this->radius = cone.radius_get();
			this->height = cone.height_get();
			this->color = cone.color_get();
			return *this;
		}
	};
}
