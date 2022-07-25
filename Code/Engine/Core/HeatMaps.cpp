#include "HeatMaps.hpp"

TileHeatMap::TileHeatMap(const IntVec2& dimensions)
	:m_dimensions(dimensions)
{
	m_values.resize(dimensions.x * dimensions.y);
}

TileHeatMap::TileHeatMap(const TileHeatMap& copyFrom)
	: m_dimensions(copyFrom.m_dimensions)
{
	m_values = copyFrom.m_values;
}

TileHeatMap::~TileHeatMap()
{
}

void TileHeatMap::SetAllValues(float value)
{
	for (size_t idx = 0; idx < m_values.size(); idx++)
	{
		m_values[idx] = value;
	}
}
