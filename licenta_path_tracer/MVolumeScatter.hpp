#pragma once
#include "Material.hpp"
#include "Color.hpp"


class MVolumeScatter : public Material {
	static int mv_scatter_index;

protected:
	Color color;
	float density;
	float diameter;

public:
	MVolumeScatter(Color color = Color::white, float density = 1.0f, float diameter = 0.1f);
	~MVolumeScatter() override;

	getset(color)
	getset(density)
	getset(diameter)
};
