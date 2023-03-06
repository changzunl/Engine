#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RaycastResult.hpp"

struct Plane2D
{
public:
    Plane2D();
    Plane2D(const Vec2& normal, float dist);
    Plane2D(const Plane2D& copyFrom);

public:
    RaycastResult2D Raycast(const Vec2& startPos, const Vec2& normal, float dist) const;
    int             GetSide(const Vec2& pos) const;

    bool operator==(const Plane2D& other) const;
    void operator=(const Plane2D& other);

public:
    float m_dist;
    Vec2  m_normal;
};

