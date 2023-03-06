#include "Engine/Math/ConvexHull2D.hpp"


ConvexHull2D::ConvexHull2D(ConvexHull2D&& moveFrom) noexcept
    : m_planes(std::move(moveFrom.m_planes))
{
}

RaycastResult2D ConvexHull2D::Raycast(const Vec2& startPos, const Vec2& normal, float dist) const
{
    if (IsPointInside(startPos))
    {
        return RaycastResult2D(0, startPos, -normal);
    }

    RaycastResult2D hit;

    for (size_t i = 0; i < m_planes.size(); i++)
    {
        auto& plane = m_planes[i];

        auto result = plane.Raycast(startPos, normal, dist);

        if (result.DidImpact() && result.GetImpactNormal().Dot(normal) > 0)
        {
            bool outside = false;

            for (size_t j = 0; j < m_planes.size(); j++)
            {
                if (i == j)
                    continue;

                if (m_planes[j].GetSide(result.GetImpactPosition()) < 0)
                {
                    outside = true;
                    break;
                }
            }

            if (outside)
                continue;

            if (!hit.DidImpact() || result.GetImpactDistance() < hit.GetImpactDistance())
            {
                hit = result;
            }
        }
    }

    return hit.DidImpact() ? RaycastResult2D(hit.GetImpactDistance(), hit.GetImpactPosition(), -hit.GetImpactNormal()) : hit;
}

bool ConvexHull2D::IsPointInside(const Vec2& pos) const
{
    for (size_t i = 0; i < m_planes.size(); i++)
    {
        if (m_planes[i].GetSide(pos) < 0)
        {
            return false;
        }
    }

    return true;
}

void ConvexHull2D::operator=(ConvexHull2D&& moveFrom) noexcept
{
    m_planes = std::move(moveFrom.m_planes);
}

