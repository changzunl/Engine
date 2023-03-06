#include "Engine/Math/RaycastResult.hpp"

bool RaycastResult2D::operator==(const RaycastResult2D& other)
{
    return
           m_didImpact    == other.m_didImpact
        && m_impactDist   == other.m_impactDist
        && m_impactPos    == other.m_impactPos
        && m_impactNormal == other.m_impactNormal;
}

void RaycastResult2D::operator=(const RaycastResult2D& other)
{
    m_didImpact    = other.m_didImpact;
    m_impactDist   = other.m_impactDist;
    m_impactPos    = other.m_impactPos;
    m_impactNormal = other.m_impactNormal;
}
