#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/ConvexHull2D.hpp"

#include <vector>


class ConvexPoly2D
{
public:
    static ConvexPoly2D MakeRandom(const Vec2& center, float radius, int vertCnt);
    ConvexPoly2D() {}
    ConvexPoly2D(const ConvexPoly2D& copyFrom);

    bool IsPointInside(const Vec2& pos);
    void Move(const Vec2& offset);
    ConvexHull2D ToHull2D() const;

    inline const std::vector<Vec2>& GetVertices() const { return m_vertices; }

    void operator=(const ConvexPoly2D& copyFrom);

private:
    std::vector<Vec2> m_vertices;
};

