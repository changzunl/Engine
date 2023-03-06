#pragma once

#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/RaycastResult.hpp"

#include <vector>


struct ConvexHull2D
{
public:
    ConvexHull2D() = default;
    ConvexHull2D(ConvexHull2D&& moveFrom) noexcept;

    RaycastResult2D Raycast(const Vec2& startPos, const Vec2& normal, float dist) const;
    bool IsPointInside(const Vec2& pos) const;

    void operator=(ConvexHull2D&& moveFrom) noexcept;

public:
    std::vector<Plane2D> m_planes;
};

