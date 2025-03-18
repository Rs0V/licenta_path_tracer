#pragma once
#include "Material.hpp"
#include "Color.hpp"


class MPrincipledBSDF : public Material {
	static int mp_bsdf_index;

protected:
	Color albedo;
	float metallic;

	float roughness;
	float ior;

	float reflectance;
	float transmission;

public:
	MPrincipledBSDF(
		Color albedo = Color::white,
		float metallic = 0.0f,
		float roughness = 0.48f,
		float ior = 1.45f,
		float reflectance = 0.5f,
		float transmission = 0.0f
	);
	~MPrincipledBSDF() override;

	getset(albedo)
	getset(metallic)

	getset(roughness)
	getset(ior)

	getset(reflectance)
	getset(transmission)
};
