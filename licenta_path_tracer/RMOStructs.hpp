// RAYMARCHING OBJECT STRUCTS
#pragma once
#include "glm.hpp"
#include "Sphere.hpp"
#include "Cube.hpp"
#include "Cylinder.hpp"
#include "Cone.hpp"
#include "Boolean.hpp"
#include "PointLight.hpp"
#include "MPrincipledBSDF.hpp"
#include "MVolumeScatter.hpp"


namespace rmo {
	int getBooleanType(const boolean::Boolean *boolean) {
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
		int visible;
		glm::vec3 location;

		int affectWorld;
		glm::vec3 rotation;

		float radius;
		glm::vec3 scale;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		privb(int pad1);



		Sphere& operator=(const ::Sphere &sphere) {
			this->location = sphere.transform_getrc().location;
			this->rotation = sphere.transform_getrc().rotation;
			this->scale    = sphere.transform_getrc().scale;

			this->radius = sphere.radius_get();

			auto boolean = vuni_cast<boolean::Boolean*>(sphere.components_getrc());
			this->booleanObjType  = boolean ? boolean->other_get()->type_get()  : sphere.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : sphere.index_get();
			this->booleanType     = getBooleanType(boolean);

			this->visible     = sphere.visible_get();
			this->affectWorld = sphere.affectWorld_get();

			return *this;
		}
	};

	struct Cube {
		int visible;
		glm::vec3 location;

		int affectWorld;
		glm::vec3 rotation;

		privb(float pad1);
		glm::vec3 scale;

		privb(float pad2);
		glm::vec3 dimensions;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		privb(int pad3);



		Cube& operator=(const ::Cube &cube) {
			this->location = cube.transform_getrc().location;
			this->rotation = cube.transform_getrc().rotation;
			this->scale    = cube.transform_getrc().scale;

			this->dimensions = cube.dimensions_get();

			auto boolean = vuni_cast<boolean::Boolean*>(cube.components_getrc());
			this->booleanObjType  = boolean ? boolean->other_get()->type_get()  : cube.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : cube.index_get();
			this->booleanType     = getBooleanType(boolean);

			this->visible     = cube.visible_get();
			this->affectWorld = cube.affectWorld_get();

			return *this;
		}
	};

	struct Cylinder {
		float radius;
		glm::vec3 location;

		float height;
		glm::vec3 rotation;

		int visible;
		glm::vec3 scale;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		int affectWorld;



		Cylinder& operator=(const ::Cylinder &cylinder) {
			this->location = cylinder.transform_getrc().location;
			this->rotation = cylinder.transform_getrc().rotation;
			this->scale    = cylinder.transform_getrc().scale;

			this->radius = cylinder.radius_get();
			this->height = cylinder.height_get();

			auto boolean = vuni_cast<boolean::Boolean*>(cylinder.components_getrc());
			this->booleanObjType  = boolean ? boolean->other_get()->type_get()  : cylinder.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : cylinder.index_get();
			this->booleanType     = getBooleanType(boolean);

			this->visible     = cylinder.visible_get();
			this->affectWorld = cylinder.affectWorld_get();

			return *this;
		}
	};

	struct Cone {
		float radius;
		glm::vec3 location;

		float height;
		glm::vec3 rotation;

		int visible;
		glm::vec3 scale;

		int booleanObjType;
		int booleanObjIndex;
		int booleanType;
		int affectWorld;



		Cone& operator=(const ::Cone &cone) {
			this->location = cone.transform_getrc().location;
			this->rotation = cone.transform_getrc().rotation;
			this->scale    = cone.transform_getrc().scale;

			this->radius = cone.radius_get();
			this->height = cone.height_get();

			auto boolean = vuni_cast<boolean::Boolean*>(cone.components_getrc());
			this->booleanObjType  = boolean ? boolean->other_get()->type_get()  : cone.type_get();
			this->booleanObjIndex = boolean ? boolean->other_get()->index_get() : cone.index_get();
			this->booleanType     = getBooleanType(boolean);

			this->visible     = cone.visible_get();
			this->affectWorld = cone.affectWorld_get();

			return *this;
		}
	};


	struct PointLight {
		float intensity;
		glm::vec3 location;

		float radius;
		glm::vec3 rotation;

		privb(float pad1);
		glm::vec3 scale;

		glm::vec4 color;



		PointLight& operator=(const ::PointLight &point_light) {
			this->location = point_light.transform_getrc().location;
			this->rotation = point_light.transform_getrc().rotation;
			this->scale    = point_light.transform_getrc().scale;

			this->intensity = point_light.intensity_get();
			this->radius    = point_light.radius_get();

			this->color = point_light.color_get();

			return *this;
		}
	};


	struct MPrincipledBSDF {
		glm::vec4 albedo;

		float metallic;
		float roughness;
		float ior;
		float reflectance;
		
		float transmission;
		privb(float pad1[3]);



		MPrincipledBSDF& operator=(const ::MPrincipledBSDF &mprincipled_bsdf) {
			this->albedo = mprincipled_bsdf.albedo_get();

			this->metallic    = mprincipled_bsdf.metallic_get();
			this->roughness   = mprincipled_bsdf.roughness_get();
			this->ior         = mprincipled_bsdf.ior_get();
			this->reflectance = mprincipled_bsdf.reflectance_get();

			this->transmission = mprincipled_bsdf.transmission_get();

			return *this;
		}
	};

	struct MVolumeScatter {
		glm::vec4 color;

		float density;
		float diameter;
		privb(float pad1[2]);



		MVolumeScatter& operator=(const ::MVolumeScatter &mvolume_scatter) {
			this->color = mvolume_scatter.color_get();

			this->density  = mvolume_scatter.density_get();
			this->diameter = mvolume_scatter.diameter_get();

			return *this;
		}
	};


	struct CBoolean {
		int selfObjType;
		int selfObjIndex;
		int otherObjType;
		int otherObjIndex;
		
		int booleanType;
		privb(int pad1[3]);



		CBoolean& operator=(const ::boolean::Boolean &boolean) {
			this->selfObjType  = boolean.self_get()->type_get();
			this->selfObjIndex = boolean.self_get()->index_get();

			this->otherObjType  = boolean.other_get()->type_get();
			this->otherObjIndex = boolean.other_get()->index_get();

			this->booleanType = getBooleanType(&boolean);

			return *this;
		}
	};
}
