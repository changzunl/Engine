#include "Engine/Math/ConvexPoly2D.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include <algorithm>


ConvexPoly2D ConvexPoly2D::MakeRandom(const Vec2& center, float radius, int vertCnt)
{
    RandomNumberGenerator rng;

    ConvexPoly2D poly;
    poly.m_vertices.reserve(vertCnt);

    std::vector<float> angles;
    angles.resize(vertCnt);

    for (auto& ang : angles)
        ang = rng.RollRandomFloatInRange(0.f, 360.f);

    std::sort(angles.begin(), angles.end(), std::greater<float>());

    for (auto& ang : angles)
        poly.m_vertices.push_back(Vec2::MakeFromPolarDegrees(ang) * radius + center);

    return poly;
}

ConvexPoly2D::ConvexPoly2D(const ConvexPoly2D& copyFrom)
    : m_vertices(copyFrom.m_vertices)
{

}

ConvexHull2D ConvexPoly2D::ToHull2D() const
{
    int size = (int) m_vertices.size();

    ConvexHull2D hull;
    hull.m_planes.reserve(size);

    for (int i = 0; i < size; i++)
    {
        auto& pos1 = m_vertices[i];
        auto& pos2 = m_vertices[(i + 1) % size];

        auto normal = (pos2 - pos1).GetNormalized().GetRotatedMinus90Degrees();

        auto dist = pos2.Dot(normal);

        hull.m_planes.emplace_back(normal, dist);
    }

    return hull;
}

bool ConvexPoly2D::IsPointInside(const Vec2& pos)
{
    bool sign = (pos - m_vertices.back()).Cross(m_vertices.front() - m_vertices.back()) > 0;

    for (int i = 0; i < m_vertices.size() - 1; i++)
    {
        bool sign2 = (pos - m_vertices[i]).Cross(m_vertices[i + 1] - m_vertices[i]) > 0;

        if (sign2 != sign)
            return false;
    }
    return true;
}

void ConvexPoly2D::Move(const Vec2& offset)
{
    for (auto& pos : m_vertices)
        pos += offset;
}

void ConvexPoly2D::operator=(const ConvexPoly2D& copyFrom)
{
    m_vertices = copyFrom.m_vertices;
}
