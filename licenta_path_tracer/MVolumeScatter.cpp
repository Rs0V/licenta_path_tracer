#include "MVolumeScatter.hpp"

int MVolumeScatter::mv_scatter_index = 0;


MVolumeScatter::MVolumeScatter(Color color, float density, float diameter)
	:
	Material(1, MVolumeScatter::mv_scatter_index++),
	color(color),
	density(density),
	diameter(diameter)
{
}

MVolumeScatter::~MVolumeScatter() {}
