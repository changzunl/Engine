#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"


struct RaycastResult2D
{
public:
    inline explicit    RaycastResult2D()                                                                     : m_didImpact(false) {};
    inline explicit    RaycastResult2D(float impactDist, const Vec2& impactPos, const Vec2& impactNormal)    : m_didImpact(true), m_impactDist(impactDist), m_impactPos(impactPos), m_impactNormal(impactNormal) {};
    inline             RaycastResult2D(const RaycastResult2D& copyFrom)                                      : m_didImpact(copyFrom.m_didImpact), m_impactDist(copyFrom.m_impactDist), m_impactPos(copyFrom.m_impactPos), m_impactNormal(copyFrom.m_impactNormal) {};

public:                                                                                               
    inline bool        DidImpact() const                                                                     { return m_didImpact; }
    inline float       GetImpactDistance() const                                                             { return m_impactDist; }
    inline const Vec2& GetImpactPosition() const                                                             { return m_impactPos; }
    inline const Vec2& GetImpactNormal() const                                                               { return m_impactNormal; }

           bool        operator==(const RaycastResult2D& other);
           void        operator=(const RaycastResult2D& copyFrom);

private:
    bool    m_didImpact = false;
    float   m_impactDist = 0.0f;
    Vec2    m_impactPos;
    Vec2    m_impactNormal;
};

struct RaycastResult3D
{
public:
    inline explicit    RaycastResult3D()                                                                     : m_didImpact(false) {};
    inline explicit    RaycastResult3D(float impactDist, const Vec3& impactPos, const Vec3& impactNormal)    : m_didImpact(true), m_impactDist(impactDist), m_impactPos(impactPos), m_impactNormal(impactNormal) {};

public:                                                                                               
    inline bool        DidImpact() const                                                                     { return m_didImpact; }
    inline float       GetImpactDistance() const                                                             { return m_impactDist; }
    inline const Vec3& GetImpactPosition() const                                                             { return m_impactPos; }
    inline const Vec3& GetImpactNormal() const                                                               { return m_impactNormal; }

private:
	bool    m_didImpact = false;
	float   m_impactDist = 0.0f;
	Vec3    m_impactPos;
	Vec3    m_impactNormal;
};
