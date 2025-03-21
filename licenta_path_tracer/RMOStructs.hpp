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
	struct Sphere {
		alignas(16) glm::vec3 location;
		int visible;

		alignas(16) glm::vec3 rotation;
		int affectWorld;

		alignas(16) glm::vec3 scale;
		float radius;

		int type;
		int index;
		int material_type;
		int material_index;


		Sphere& operator=(const ::Sphere &sphere) {
			this->location = sphere.transform_getrc().location;
			this->rotation = sphere.transform_getrc().rotation;
			this->scale    = sphere.transform_getrc().scale;

			this->radius = sphere.radius_get();

			this->type  = sphere.type_get();
			this->index = sphere.index_get();
			this->material_type  = sphere.material_get()->type_get();
			this->material_index = sphere.material_get()->index_get();

			this->visible     = sphere.visible_get();
			this->affectWorld = sphere.affectWorld_get();

			return *this;
		}
	};

	struct Cube {
		alignas(16) glm::vec3 location;
		int visible;

		alignas(16) glm::vec3 rotation;
		int affectWorld;

		alignas(16) glm::vec3 scale;
		int material_type;

		alignas(16) glm::vec3 dimensions;
		int material_index;

		int type;
		int index;


		Cube& operator=(const ::Cube &cube) {
			this->location = cube.transform_getrc().location;
			this->rotation = cube.transform_getrc().rotation;
			this->scale    = cube.transform_getrc().scale;

			this->dimensions = cube.dimensions_get();

			this->type  = cube.type_get();
			this->index = cube.index_get();
			this->material_type  = cube.material_get()->type_get();
			this->material_index = cube.material_get()->index_get();

			this->visible     = cube.visible_get();
			this->affectWorld = cube.affectWorld_get();

			return *this;
		}
	};

	struct Cylinder {
		alignas(16) glm::vec3 location;
		float radius;

		alignas(16) glm::vec3 rotation;
		float height;

		alignas(16) glm::vec3 scale;
		int visible;

		int affectWorld;
		int type;
		int index;
		int material_type;

		int material_index;


		Cylinder& operator=(const ::Cylinder &cylinder) {
			this->location = cylinder.transform_getrc().location;
			this->rotation = cylinder.transform_getrc().rotation;
			this->scale    = cylinder.transform_getrc().scale;

			this->radius = cylinder.radius_get();
			this->height = cylinder.height_get();

			this->type  = cylinder.type_get();
			this->index = cylinder.index_get();
			this->material_type = cylinder.material_get()->type_get();
			this->material_index = cylinder.material_get()->index_get();

			this->visible     = cylinder.visible_get();
			this->affectWorld = cylinder.affectWorld_get();

			return *this;
		}
	};

	struct Cone {
		alignas(16) glm::vec3 location;
		float radius;

		alignas(16) glm::vec3 rotation;
		float height;

		alignas(16) glm::vec3 scale;
		int visible;

		int affectWorld;
		int type;
		int index;
		int material_type;

		int material_index;


		Cone& operator=(const ::Cone &cone) {
			this->location = cone.transform_getrc().location;
			this->rotation = cone.transform_getrc().rotation;
			this->scale    = cone.transform_getrc().scale;

			this->radius = cone.radius_get();
			this->height = cone.height_get();

			this->type  = cone.type_get();
			this->index = cone.index_get();
			this->material_type = cone.material_get()->type_get();
			this->material_index = cone.material_get()->index_get();

			this->visible     = cone.visible_get();
			this->affectWorld = cone.affectWorld_get();

			return *this;
		}
	};


	struct CBoolean {
		int selfObjType;
		int selfObjIndex;
		int otherObjType;
		int otherObjIndex;

		int type;
		float blend;


		CBoolean& operator=(const ::boolean::Boolean& boolean) {
			this->selfObjType = boolean.self_get()->type_get();
			this->selfObjIndex = boolean.self_get()->index_get();

			this->otherObjType = boolean.other_get()->type_get();
			this->otherObjIndex = boolean.other_get()->index_get();

			this->type = boolean.type_get();
			this->blend = boolean.blend_get();

			return *this;
		}
	};


	struct PointLight {
		alignas(16) glm::vec3 location;
		float intensity;

		alignas(16) glm::vec3 color;
		float radius;


		PointLight& operator=(const ::PointLight &point_light) {
			this->location = point_light.transform_getrc().location;

			this->intensity = point_light.intensity_get();
			this->radius    = point_light.radius_get();

			this->color = point_light.color_get();

			return *this;
		}
	};


	struct MPrincipledBSDF {
		alignas(16) glm::vec3 albedo;
		float metallic;

		float roughness;
		float ior;
		float reflectance;
		float transmission;


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
		alignas(16) glm::vec3 color;
		float density;

		float diameter;


		MVolumeScatter& operator=(const ::MVolumeScatter &mvolume_scatter) {
			this->color = mvolume_scatter.color_get();

			this->density  = mvolume_scatter.density_get();
			this->diameter = mvolume_scatter.diameter_get();

			return *this;
		}
	};
}
