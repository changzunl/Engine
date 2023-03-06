#include "Engine/Math/Plane2D.hpp"

#include <cmath>


Plane2D::Plane2D() : Plane2D(Vec2::ZERO, 0)
{
}

Plane2D::Plane2D(const Vec2& normal, float dist)
    : m_normal(normal)
    , m_dist(dist)
{
}

Plane2D::Plane2D(const Plane2D& copyFrom)
    : m_normal(copyFrom.m_normal)
    , m_dist(copyFrom.m_dist)
{

}

RaycastResult2D Plane2D::Raycast(const Vec2& startPos, const Vec2& normal, float dist) const
{
    float projNormal = normal.Dot(m_normal);

    if (fabsf(projNormal) < 0.00001f) // parallel
        return RaycastResult2D();

    float distStart = startPos.Dot(m_normal);

    if ((projNormal > 0 && distStart > m_dist) || (projNormal < 0 && distStart < m_dist))
        return RaycastResult2D();

    float projImpact = m_dist - distStart;
    float impactDist = projImpact / projNormal;

    if (impactDist < 0 || impactDist > dist)
        return RaycastResult2D();

    return RaycastResult2D(impactDist, startPos + normal * impactDist, m_normal);
}

int Plane2D::GetSide(const Vec2& pos) const
{
    float dist = pos.Dot(m_normal);
    return dist > m_dist ? 1 : -1;
}

void Plane2D::operator=(const Plane2D& other)
{
    m_normal = other.m_normal;
    m_dist = other.m_dist;
}

bool Plane2D::operator==(const Plane2D& other) const
{
    return m_normal == other.m_normal && m_dist == other.m_dist;
}
