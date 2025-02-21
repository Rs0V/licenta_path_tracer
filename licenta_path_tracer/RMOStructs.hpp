// RAYMARCHING OBJECT STRUCTS
#pragma once
#include "glm.hpp"
#include "Sphere.hpp"
#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Cone.hpp"
#include "Boolean.hpp"
#include "tuple"


namespace rmo {
	int getBooleanType(boolean::Boolean* boolean) {
		if (boolean == nullptr) {
			return -1;
		}
		switch (boolean->type_get()) {
		case boolean::Type::Union:
			return 0;
		case boolean::Type::Intersect:
			return 1;
		case boolean::Type::Difference:
			return 2;
		}
	}

	struct Sphere {
		glm::vec3 location;
		privb(float pad0);

		glm::vec3 rotation;
		privb(float pad1);

		glm::vec3 scale;
		privb(float pad2);

		float radius;
		privb(float pad3[3]);

		glm::vec4 color;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		privb(int pad4);


		Sphere& operator=(const ::Sphere &sphere) {
			this->location = sphere.transform_getrc().location;
			this->rotation = sphere.transform_getrc().rotation;
			this->scale = sphere.transform_getrc().scale;

			this->radius = sphere.radius_get();

			this->color = sphere.color_get();

			auto boolean = vuni_cast<boolean::Boolean*>(sphere.components_getrc());
			this->booleanObjType = boolean ? boolean->other_get()->type_get() : sphere.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : sphere.index_get();
			this->booleanType = getBooleanType(boolean);

			return *this;
		}
	};

	struct Cube {
		glm::vec3 location;
		privb(float pad0)

		glm::vec3 rotation;
		privb(float pad1)

		glm::vec3 scale;
		privb(float pad2)

		glm::vec3 dimensions;
		privb(float pad3)

		glm::vec4 color;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		privb(int pad4);


		Cube& operator=(const ::Cube& cube) {
			this->location = cube.transform_getrc().location;
			this->rotation = cube.transform_getrc().rotation;
			this->scale = cube.transform_getrc().scale;

			this->dimensions = cube.dimensions_get();

			this->color = cube.color_get();

			auto boolean = vuni_cast<boolean::Boolean*>(cube.components_getrc());
			this->booleanObjType = boolean ? boolean->other_get()->type_get() : cube.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : cube.index_get();
			this->booleanType = getBooleanType(boolean);

			return *this;
		}
	};

	struct Cylinder {
		glm::vec3 location;
		privb(float pad0)

		glm::vec3 rotation;
		privb(float pad1)

		glm::vec3 scale;
		privb(float pad2)

		float radius;
		privb(float pad3[3])

		float height;
		privb(float pad4[3])

		glm::vec4 color;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		privb(int pad5);


		Cylinder& operator=(const ::Cylinder& cylinder) {
			this->location = cylinder.transform_getrc().location;
			this->rotation = cylinder.transform_getrc().rotation;
			this->scale = cylinder.transform_getrc().scale;

			this->radius = cylinder.radius_get();
			this->height = cylinder.height_get();

			this->color = cylinder.color_get();

			auto boolean = vuni_cast<boolean::Boolean*>(cylinder.components_getrc());
			this->booleanObjType = boolean ? boolean->other_get()->type_get() : cylinder.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : cylinder.index_get();
			this->booleanType = getBooleanType(boolean);

			return *this;
		}
	};

	struct Cone {
		glm::vec3 location;
		privb(float pad0)

		glm::vec3 rotation;
		privb(float pad1)

		glm::vec3 scale;
		privb(float pad2)

		float radius;
		privb(float pad3[3])

		float height;
		privb(float pad4[3])

		glm::vec4 color;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		privb(int pad5);


		Cone& operator=(const ::Cone& cone) {
			this->location = cone.transform_getrc().location;
			this->rotation = cone.transform_getrc().rotation;
			this->scale = cone.transform_getrc().scale;

			this->radius = cone.radius_get();
			this->height = cone.height_get();

			this->color = cone.color_get();

			auto boolean = vuni_cast<boolean::Boolean*>(cone.components_getrc());
			this->booleanObjType = boolean ? boolean->other_get()->type_get() : cone.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : cone.index_get();
			this->booleanType = getBooleanType(boolean);

			return *this;
		}
	};
}
