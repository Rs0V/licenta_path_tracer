#include "MPrincipledBSDF.hpp"

int MPrincipledBSDF::mp_bsdf_index = 0;


MPrincipledBSDF::MPrincipledBSDF(
	Color albedo,
	float metallic,
	float roughness,
	float ior,
	float reflectance,
	float transmission,
	Color emissive
) :
	Material(0, MPrincipledBSDF::mp_bsdf_index++),
	albedo(albedo),
	metallic(metallic),
	roughness(roughness),
	ior(ior),
	reflectance(reflectance),
	transmission(transmission),
	emissive(emissive)
{
}

MPrincipledBSDF::~MPrincipledBSDF() {}
