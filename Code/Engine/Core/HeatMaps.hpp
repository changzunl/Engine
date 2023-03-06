#pragma once

#include "Engine/Math/IntVec2.hpp"
#include <vector>

template<typename T> 
class GridMap
{
public:
    GridMap(const IntVec2& dimensions, T value = T())
        : m_dimensions(dimensions)
        , m_values(dimensions.x * (size_t)dimensions.y, value)
    {
    }

    GridMap(const GridMap<T>& copyFrom)
        : m_dimensions(copyFrom.m_dimensions)
        , m_values(copyFrom.m_values)
    {
    }

    GridMap(GridMap<T>&& moveFrom) noexcept
        : m_dimensions(std::move(moveFrom.m_dimensions))
        , m_values(std::move(moveFrom.m_values))
    {
    }

    ~GridMap() {}

    GridMap<T>& operator=(const GridMap<T>& copyFrom)
    {
        m_dimensions = copyFrom.m_dimensions;
        m_values = copyFrom.m_values;

        return *this;
    }

    GridMap<T>& operator=(GridMap<T>&& copyFrom) noexcept
    {
        m_dimensions = std::move(copyFrom.m_dimensions);
        m_values = std::move(copyFrom.m_values);

        return *this;
    }

    const IntVec2&         GetDimensions() const { return m_dimensions; }
    const T&               GetValue(const IntVec2& coord) const { return GetValueRefAt(coord.x, coord.y); }
    const T&               GetValue(int coordX, int coordY) const { return GetValueRefAt(coordX, coordY); }
    void                   SetValue(const IntVec2& coord, const T& value) { GetValueRefAt(coord.x, coord.y) = value; }
    void                   SetValue(int coordX, int coordY, const T& value) { GetValueRefAt(coordX, coordY) = value; }
    void                   AddValue(const IntVec2& coord, const T& value) { GetValueRefAt(coord.x, coord.y) += value; }
    void                   AddValue(int coordX, int coordY, const T& value) { GetValueRefAt(coordX, coordY) += value; }
    void                   SetAllValues(const T& value) { m_values.assign(m_values.size(), value); }

private:
    const T& GetValueRefAt(int coordX, int coordY) const { return m_values[coordX + coordY * (size_t)m_dimensions.x]; }
    T& GetValueRefAt(int coordX, int coordY) { return m_values[coordX + coordY * (size_t)m_dimensions.x]; }

private:
    IntVec2            m_dimensions;
    std::vector<T>     m_values;
};

typedef  GridMap<float> TileHeatMap;
typedef  GridMap<unsigned char> CompressedHeatMap;
