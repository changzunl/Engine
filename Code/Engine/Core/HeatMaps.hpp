#pragma once

#include "Engine/Math/IntVec2.hpp"
#include <vector>

class TileHeatMap
{
public:
    TileHeatMap(const IntVec2& dimensions);
    TileHeatMap(const TileHeatMap& copyFrom);
    ~TileHeatMap();

    const IntVec2&         GetDimensions() const                                       { return m_dimensions; }
    float                  GetValue(const IntVec2& coord) const                        { return GetValueRefAt(coord.x, coord.y); }
    float                  GetValue(int coordX, int coordY) const                      { return GetValueRefAt(coordX, coordY); }
    void                   SetValue(const IntVec2& coord, float value)                 { GetValueRefAt(coord.x, coord.y) = value; }
    void                   SetValue(int coordX, int coordY, float value)               { GetValueRefAt(coordX, coordY) = value; }
    void                   AddValue(const IntVec2& coord, float value)                 { GetValueRefAt(coord.x, coord.y) += value; }
    void                   AddValue(int coordX, int coordY, float value)               { GetValueRefAt(coordX, coordY) += value; }
    void                   SetAllValues(float value);

private:
    const float&           GetValueRefAt(int coordX, int coordY) const                 { return m_values[coordX + coordY * m_dimensions.x]; }
    float&                 GetValueRefAt(int coordX, int coordY)                       { return m_values[coordX + coordY * m_dimensions.x]; }

private:
    IntVec2            m_dimensions;
    std::vector<float> m_values;
};

