#include "MPrincipledBSDF.hpp"

int MPrincipledBSDF::mp_bsdf_index = 0;


MPrincipledBSDF::MPrincipledBSDF(
	Color albedo,
	float metallic,
	float roughness,
	float ior,
	float reflectance,
	float transmission
) :
	Material(0, MPrincipledBSDF::mp_bsdf_index++),
	albedo(albedo),
	metallic(metallic),
	roughness(roughness),
	ior(ior),
	reflectance(reflectance),
	transmission(transmission)
{
}

MPrincipledBSDF::~MPrincipledBSDF() {}
