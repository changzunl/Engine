#include "MathUtils.hpp"

#include "RandomNumberGenerator.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "IntVec2.hpp"
#include "AABB2.hpp"
#include "AABB3.hpp"
#include "OBB2.hpp"
#include "LineSegment2.hpp"
#include "Plane2D.hpp"
#include "Capsule2.hpp"
#include "EulerAngles.hpp"
#include "FloatRange.hpp"
#include <math.h>


void ClampValue(float& value, float lower, float upper)
{
    value = value < lower ? lower : (value > upper ? upper : value);
}

void ClampIntValue(int& value, int lower, int upper)
{
    value = value < lower ? lower : (value > upper ? upper : value);
}

float ClampZeroToOne(float input)
{
    return Clamp(input, 0.f, 1.f);
}

void ClampValueZeroToOne(float& value)
{
    ClampValue(value, 0.f, 1.f);
}

float SquareRoot(float input)
{
    return sqrtf(input);
}

int Floor(float input)
{
    return static_cast<int>(floorf(input));
}

int Ceil(float input)
{
    return static_cast<int>(ceilf(input));
}

float Lerp(float start, float end, float fraction)
{
    return start * (1 - fraction) + end * fraction;
}

Vec3 Lerp(const Vec3& point1, const Vec3& point2, float fraction)
{
    return Vec3(Lerp(point1.x, point2.x, fraction), Lerp(point1.y, point2.y, fraction), Lerp(point1.z, point2.z, fraction));
}

Vec2 Lerp(const Vec2& point1, const Vec2& point2, float fraction)
{
    return Vec2(Lerp(point1.x, point2.x, fraction), Lerp(point1.y, point2.y, fraction));
}

float GetFractionWithin(float value, float start, float end)
{
    float dispUnit = end - start;
    if (dispUnit == 0) return 0.5f;

    float dispInput = value - start;
    return dispInput / dispUnit;
}

float RangeMap(float rawValue, float inRangeStart, float inRangeEnd, float outRangeStart, float outRangeEnd)
{
    float fraction = GetFractionWithin(rawValue, inRangeStart, inRangeEnd);
    return Lerp(outRangeStart, outRangeEnd, fraction);
}

float RangeMapClamped(float rawValue, float inRangeStart, float inRangeEnd, float outRangeStart, float outRangeEnd)
{
    float outRangeMin = Min(outRangeStart, outRangeEnd);
    float outRangeMax = Max(outRangeStart, outRangeEnd);
    return Clamp(RangeMap(rawValue, inRangeStart, inRangeEnd, outRangeStart, outRangeEnd), outRangeMin, outRangeMax);
}

float NormalizeByte(unsigned char byteValue)
{
    return static_cast<float>(byteValue) / 255.0f;
}

unsigned char DenormalizeByte(float floatValue)
{
    return static_cast<unsigned char>(floatValue * 255.999f);
}

float ConvertDegreesToRadians(float degrees)
{
    return degrees / 180.f * PI;
}

float ConvertRadiansToDegrees(float radians)
{
    return radians / PI * 180.f;
}

float CosDegrees(float degrees)
{
    return cosf(ConvertDegreesToRadians(degrees));
}

float SinDegrees(float degrees)
{
    return sinf(ConvertDegreesToRadians(degrees));
}

float Atan2Degrees(float y, float x)
{
    return ConvertRadiansToDegrees(atan2f(y, x));
}

float GetDistance2D(const Vec2& posA, const Vec2& posB)
{
    return sqrtf(GetDistanceSquared2D(posA, posB));
}

float GetDistanceSquared2D(const Vec2& posA, const Vec2& posB)
{
    Vec2 dir = posB - posA;
    return dir.GetLengthSquared();
}

float GetDistance3D(const Vec3& posA, const Vec3& posB)
{
    return sqrtf(GetDistanceSquared3D(posA, posB));
}

float GetDistanceSquared3D(const Vec3& posA, const Vec3& posB)
{
    Vec3 dir = posB - posA;
    return dir.GetLengthSquared();
}

float GetDistanceXY3D(const Vec3& posA, const Vec3& posB)
{
    return sqrtf(GetDistanceXYSquared3D(posA, posB));
}

float GetDistanceXYSquared3D(const Vec3& posA, const Vec3& posB)
{
    Vec3 dir = posB - posA;
    return dir.GetLengthXYSquared();
}

int GetTaxicabDistance2D(const IntVec2& pointA, const IntVec2& pointB)
{
    return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

float DotProduct2D(const Vec2& a, const Vec2& b)
{
    return a.Dot(b);
}

float DotProduct3D(const Vec3& a, const Vec3& b)
{
    return a.Dot(b);
}

float DotProduct4D(const Vec4& a, const Vec4& b)
{
	return a.Dot(b);
}

float CrossProduct2D(const Vec2& a, const Vec2& b)
{
    return a.Cross(b);
}

const Vec3 CrossProduct3D(const Vec3& a, const Vec3& b)
{
	return a.Cross(b);
}

float GetProjectedLength2D(const Vec2& vectorToProject, const Vec2& vectorToProjectOnto)
{
    return DotProduct2D(vectorToProject, vectorToProjectOnto.GetNormalized());
}

float GetProjectedLength3D(const Vec3& vectorToProject, const Vec3& vectorToProjectOnto)
{
    return DotProduct3D(vectorToProject, vectorToProjectOnto.GetNormalized());
}

const Vec2 GetProjectedOnto2D(const Vec2& vectorToProject, const Vec2& vectorToProjectOnto)
{
    Vec2 vectorToProjectOntoHat = vectorToProjectOnto.GetNormalized();
    float projectedLength = DotProduct2D(vectorToProject, vectorToProjectOntoHat);
    return projectedLength * vectorToProjectOntoHat;
}

const Vec3 GetProjectedOnto3D(const Vec3& vectorToProject, const Vec3& vectorToProjectOnto)
{
	Vec3 vectorToProjectOntoHat = vectorToProjectOnto.GetNormalized();
	float projectedLength = DotProduct3D(vectorToProject, vectorToProjectOntoHat);
	return projectedLength * vectorToProjectOntoHat;
}

float GetAngleDegreesBetweenVectors2D(const Vec2& pointA, const Vec2& pointB)
{
    float cosine = DotProduct2D(pointA.GetNormalized(), pointB.GetNormalized());
    return ConvertRadiansToDegrees(acosf(cosine));
}

bool IsPointInsideDisc2D(const Vec2& point, const Vec2& discCenter, float discRadius)
{
    return GetDistanceSquared2D(point, discCenter) < discRadius * discRadius;
}

bool IsPointInsideSphere(const Vec3& point, const Vec3& sphereCenter, float sphereRadius)
{
	return (sphereCenter - point).GetLengthSquared() < sphereRadius * sphereRadius;
}

bool IsPointInsideAABB3D(const Vec3& point, const AABB3& aabb)
{
	return aabb.IsPointInside(point);
}

bool IsPointInsideZCylinder(const Vec3& point, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
    if (point.z <= cylinderBottomCenter.z || point.z >= cylinderBottomCenter.z + cylinderHeight)
    {
        return false;
    }

    Vec3 centerToPos = cylinderBottomCenter;
    return centerToPos.GetLengthXYSquared() < cylinderRadius * cylinderRadius;
}

bool IsPointInsideAABB2D(const Vec2& point, const AABB2& aabb)
{
    return aabb.IsPointInside(point);
}

bool IsPointInsideOBB2D(const Vec2& point, const OBB2& obb)
{
    return obb.IsPointInside(point);
}

bool IsPointInsideCapsule2D(const Vec2& point, const Capsule2& capsule)
{
    return capsule.IsPointInside(point);
}

bool IsPointInsideCapsule2D(const Vec2& point, const Vec2& boneStartPos, const Vec2& boneEndPos, float radius)
{
    return IsPointInsideCapsule2D(point, Capsule2(boneStartPos, boneEndPos, radius));
}

bool IsPointInsideOrientedSector2D(const Vec2& point, const Vec2& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius)
{
    return IsPointInsideDirectedSector2D(point, sectorTip, Vec2::MakeFromPolarDegrees(sectorForwardDegrees), sectorApertureDegrees, sectorRadius);
}

bool IsPointInsideDirectedSector2D(const Vec2& point, const Vec2& sectorTip, const Vec2& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
    if (!IsPointInsideDisc2D(point, sectorTip, sectorRadius)) return false;

    return GetAngleDegreesBetweenVectors2D(point - sectorTip, sectorForwardNormal) < sectorApertureDegrees * 0.5f;
}

bool DoDiscsOverlap(const Vec2& centerA, float radiusA, const Vec2& centerB, float radiusB)
{
    float radius = radiusA + radiusB;
    return (centerA - centerB).GetLengthSquared() < radius * radius;
}

bool DoSpheresOverlap(const Vec3& posA, float radiusA, const Vec3& posB, float radiusB)
{
    float radius = radiusA + radiusB;
    return GetDistanceSquared3D(posA, posB) < radius * radius;
}

bool DoAABB2Overlap(const AABB2& A, const AABB2& B)
{
	if (B.m_mins.x >= A.m_maxs.x) return false;
	if (A.m_mins.x >= B.m_maxs.x) return false;
	if (B.m_mins.y >= A.m_maxs.y) return false;
	if (A.m_mins.y >= B.m_maxs.y) return false;
    return true;
}

bool DoDiscOverlapAABB2(const Vec2& discCenter, float discRadius, const AABB2& aabb)
{
    return GetDistanceSquared2D(GetNearestPointOnAABB2D(discCenter, aabb), discCenter) < discRadius * discRadius;
}

bool DoDiscOverlapOBB2(const Vec2& discCenter, float discRadius, const OBB2& obb)
{
	return GetDistanceSquared2D(GetNearestPointOnOBB2D(discCenter, obb), discCenter) < discRadius * discRadius;
}

bool DoDiscOverlapCapsule2(const Vec2& discCenter, float discRadius, const Capsule2& capsule)
{
    return GetDistanceSquared2D(GetNearestPointOnCapsule2D(discCenter, capsule), discCenter) < discRadius * discRadius;
}

bool DoAABB3Overlap(const AABB3& A, const AABB3& B)
{
	if (B.m_mins.x >= A.m_maxs.x) return false;
	if (A.m_mins.x >= B.m_maxs.x) return false;
	if (B.m_mins.y >= A.m_maxs.y) return false;
	if (A.m_mins.y >= B.m_maxs.y) return false;
	if (B.m_mins.z >= A.m_maxs.z) return false;
	if (A.m_mins.z >= B.m_maxs.z) return false;
	return true;
}

bool DoZCylinderOverlap(const Vec3& aCenter, float aRadius, float aHeight, const Vec3& bCenter, float bRadius, float bHeight)
{
    Vec3 AToB = bCenter - aCenter;
    if (AToB.GetLengthXYSquared() >= (aRadius + bRadius) * (aRadius + bRadius))
    {
        return false;
    }

    if (aCenter.z > bCenter.z + bHeight)
    {
        return false;
    }

	if (bCenter.z > aCenter.z + aHeight)
	{
		return false;
	}

    return true;
}

bool DoAABB3OverlapSphere(const AABB3& aabb, const Vec3& sphereCenter, float sphereRadius)
{
    return IsPointInsideSphere(GetNearestPointOnAABB3D(sphereCenter, aabb), sphereCenter, sphereRadius);
}

bool DoZCylinderOverlapSphere(const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight, const Vec3& sphereCenter, float sphereRadius)
{
	return IsPointInsideSphere(GetNearestPointOnZCylinder(sphereCenter, cylinderBottomCenter, cylinderRadius, cylinderHeight), sphereCenter, sphereRadius);
}

bool DoAABB3OverlapZCylinder(const AABB3& aabb, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
    Vec3 nearest = GetNearestPointOnAABB3D(cylinderBottomCenter, aabb);

    Vec3 centerToNearest = nearest - cylinderBottomCenter;
    if (centerToNearest.GetLengthXYSquared() >= cylinderRadius * cylinderRadius)
    {
        return false;
    }

	if (cylinderBottomCenter.z > aabb.m_maxs.z)
	{
		return false;
	}

	if (aabb.m_mins.z > cylinderBottomCenter.z + cylinderHeight)
	{
		return false;
	}

    return true;
}

const Vec2 GetNearestPointOnDisc2D(const Vec2& referencePos, const Vec2& discCenter, float discRadius)
{
    Vec2 disp = referencePos - discCenter;
    if (disp.GetLengthSquared() <= discRadius * discRadius) return referencePos;

    disp.SetLength(discRadius);
    return discCenter + disp;
}

const Vec2 GetNearestPointOnInfiniteLine2D(const Vec2& referencePos, const LineSegment2& line)
{
    return line.GetNearestPointOnInfiniteLine(referencePos);
}

const Vec2 GetNearestPointOnInfiniteLine2D(const Vec2& referencePos, const Vec2& pointAOnLine, const Vec2& pointBOnLine)
{
    return GetNearestPointOnInfiniteLine2D(referencePos, LineSegment2(pointAOnLine, pointBOnLine));
}

const Vec2 GetNearestPointOnLineSegmen2D(const Vec2& referencePos, const LineSegment2& lineSegment)
{
    return lineSegment.GetNearestPoint(referencePos);
}

const Vec2 GetNearestPointOnLineSegmen2D(const Vec2& referencePos, const Vec2& pointAOnLine, const Vec2& pointBOnLine)
{
    return GetNearestPointOnLineSegmen2D(referencePos, LineSegment2(pointAOnLine, pointBOnLine));
}

const Vec2 GetNearestPointOnAABB2D(const Vec2& referencePos, const AABB2& aabb)
{
    return aabb.GetNearestPoint(referencePos);
}

const Vec2 GetNearestPointOnOBB2D(const Vec2& referencePos, const OBB2& obb)
{
    return obb.GetNearestPoint(referencePos);
}

const Vec2 GetNearestPointOnCapsule2D(const Vec2& referencePos, const Capsule2& capsule)
{
    return capsule.GetNearestPoint(referencePos);
}

const Vec2 GetNearestPointOnCapsule2D(const Vec2& referencePos, const Vec2& boneStartPos, const Vec2& boneEndPos, float radius)
{
    return GetNearestPointOnCapsule2D(referencePos, Capsule2(boneStartPos, boneEndPos, radius));
}

const Vec3 GetNearestPointOnSphere(const Vec3& referencePos, const Vec3& sphereCenter, float sphereRadius)
{
	Vec3 disp = referencePos - sphereCenter;
	if (disp.GetLengthSquared() <= sphereRadius * sphereRadius) return referencePos;

	disp = disp.GetNormalized() * sphereRadius;
	return sphereCenter + disp;
}

const Vec3 GetNearestPointOnAABB3D(const Vec3& referencePos, const AABB3& aabb)
{
    return aabb.GetNearestPoint(referencePos);
}

const Vec3 GetNearestPointOnZCylinder(const Vec3& referencePos, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
    Vec3 centerToPos = referencePos - cylinderBottomCenter;
    if (centerToPos.GetLengthXYSquared() < cylinderRadius * cylinderRadius)
    {
        Vec3 result = referencePos;
        result.z = Clamp(result.z, cylinderBottomCenter.z, cylinderBottomCenter.z + cylinderHeight);
        return result;
    }
    else
	{
        centerToPos.z = 0.0f;
		Vec3 result = cylinderBottomCenter + centerToPos.GetNormalized() * cylinderRadius;
        result.z = Clamp(referencePos.z, cylinderBottomCenter.z, cylinderBottomCenter.z + cylinderHeight);
        return result;
    }
}

bool PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, const Vec2& fixedPoint)
{
    if (IsPointInsideDisc2D(fixedPoint, mobileDiscCenter, discRadius))
    {
        Vec2 disp = fixedPoint - mobileDiscCenter;
        Vec2 idealDisp(discRadius, 0);
        if (disp.GetLengthSquared() != 0)
        {
            idealDisp.SetOrientationDegrees(disp.GetOrientationDegrees());
        }
        mobileDiscCenter += disp - idealDisp;
        return true;
    }
    else
    {
        return false;
    }
}

bool PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileRadius, const Vec2& fixedDiscCenter, float fixedRadius)
{
    Vec2 distance = mobileDiscCenter - fixedDiscCenter;
    float overlap = (mobileRadius + fixedRadius) - distance.GetLength();
    if (overlap > 0)
    {
        mobileDiscCenter += distance.GetNormalized() * overlap;
        return true;
    }
    else
    {
        return false;
    }
}

bool PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius)
{
	Vec2 distance = aCenter - bCenter; // b -> a
	float lengthSq = distance.GetLengthSquared();
	float radius = aRadius + bRadius;

	if (lengthSq >= radius * radius)
		return false;

	float overlap = radius - distance.NormalizeAndGetPreviousLength();
	aCenter += distance * overlap * 0.5f;
	bCenter -= distance * overlap * 0.5f;
	return true;
}

bool PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, const AABB2& fixedBox)
{
    Vec2 nearest = fixedBox.GetNearestPoint(mobileDiscCenter);
    return PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, nearest);
}

bool PushDiscOutOfOBB2D(Vec2& mobileDiscCenter, float discRadius, const OBB2& fixedBox)
{
	Vec2 nearest = fixedBox.GetNearestPoint(mobileDiscCenter);
	return PushDiscOutOfPoint2D(mobileDiscCenter, discRadius, nearest);
}

bool BounceDiscOffEachOther(Vec2& pos1, float radius1, Vec2& vel1, float bounceness1, Vec2& pos2, float radius2, Vec2& vel2, float bounceness2)
{
	Vec2 distance = pos1 - pos2; // b -> a
	float lengthSq = distance.GetLengthSquared();
	float radius = radius1 + radius2;
	if (lengthSq >= radius * radius)
		return false;

	float overlap = radius - distance.NormalizeAndGetPreviousLength();
	pos1 += distance * overlap * 0.5f;
	pos2 -= distance * overlap * 0.5f;

	float bounceness = bounceness1 * bounceness2;
    float dot1 = distance.Dot(vel1);
    float dot2 = distance.Dot(vel2);
	if (dot1 < dot2)
	{
		Vec2 reflect1 = dot1 * distance;
		Vec2 reflect2 = dot2 * distance;
		Vec2 preserv1 = vel1 - reflect1;
		Vec2 preserv2 = vel2 - reflect2;

		vel1 = preserv1 + bounceness * reflect2;
		vel2 = preserv2 + bounceness * reflect1;
	}
	return true;
}

bool BounceDiscOffDisc(Vec2& pos1, float radius1, Vec2& vel1, float bounceness1, const Vec2& pos2, float radius2, float bounceness2)
{
	if (!DoDiscsOverlap(pos1, radius1, pos2, radius2))
	{
		return false;
	}

	float bounceness = bounceness1 * bounceness2;
	Vec2 normal = (pos1 - pos2).GetNormalized();
	PushDiscOutOfDisc2D(pos1, radius1, pos2, radius2);

	vel1 -= bounceness * 2.0f * DotProduct2D(vel1, normal) * normal;
	return true;
}

bool BounceDiscOffOBB2(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileBounceness, const OBB2& fixedBox, float fixedBounceness)
{
	if (!DoDiscOverlapOBB2(mobileCenter, mobileRadius, fixedBox))
	{
		return false;
	}

	float bounceness = mobileBounceness * fixedBounceness;
	Vec2 nearest = fixedBox.GetNearestPoint(mobileCenter);
    Vec2 normal = (mobileCenter - nearest).GetNormalized();
	PushDiscOutOfPoint2D(mobileCenter, mobileRadius, nearest);

	mobileVelocity -= bounceness * 2.0f * DotProduct2D(mobileVelocity, normal) * normal;
	return true;
}

bool BounceDiscOffCapsule(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileBounceness, const Capsule2& fixedCapsule, float fixedBounceness)
{
	if (!DoDiscOverlapCapsule2(mobileCenter, mobileRadius, fixedCapsule))
	{
		return false;
	}

	float bounceness = mobileBounceness * fixedBounceness;
	Vec2 nearest = fixedCapsule.m_bone.GetNearestPoint(mobileCenter);
	Vec2 normal = (mobileCenter - nearest).GetNormalized();
	PushDiscOutOfPoint2D(mobileCenter, mobileRadius + fixedCapsule.m_radius, nearest);

	mobileVelocity -= bounceness * 2.0f * DotProduct2D(mobileVelocity, normal) * normal;
	return true;
}

float GetShortestAngularDispDegrees(float fromDegrees, float toDegrees)
{
    float angleDispDeg = toDegrees - fromDegrees;
    if (angleDispDeg > 0)
    {
        angleDispDeg = fmodf(angleDispDeg, 360.f);
        if (angleDispDeg > 180.f)
        {
            angleDispDeg -= 360.f;
        }
    }
    else if (angleDispDeg < 0)
    {
        angleDispDeg = -fmodf(-angleDispDeg, 360.f);
        if (angleDispDeg < -180.f)
        {
            angleDispDeg += 360.f;
        }
    }
    return angleDispDeg;
}

float GetTurnedTowardDegrees(float fromDegrees, float toDegrees, float maxTurnRateDegrees)
{
    float angleDispDeg = GetShortestAngularDispDegrees(fromDegrees, toDegrees);
    if (fabsf(angleDispDeg) <= maxTurnRateDegrees)
    {
        return toDegrees;
    }
    return fromDegrees + Clamp(angleDispDeg, -maxTurnRateDegrees, maxTurnRateDegrees);
}

void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, const Vec2& translate)
{
    Vec2 temp = Vec2(posToTransform);

    // Scale
    temp *= uniformScale;

    // Rotate
    temp.RotateDegrees(rotationDegrees);

    // Transform
    temp += translate;

    posToTransform.x = temp.x;
    posToTransform.y = temp.y;
}

void TransformPosition2D(Vec2& posToTransform, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translate)
{
    posToTransform = posToTransform.x * iBasis + posToTransform.y * jBasis + translate;
}

void TransformPositionXY3D(Vec3& posToTransform, float uniformScale, float rotationDegrees, const Vec2& translateXY)
{
    Vec2 temp = Vec2(posToTransform.x, posToTransform.y);
    TransformPosition2D(temp, uniformScale, rotationDegrees, translateXY);
    posToTransform.x = temp.x;
    posToTransform.y = temp.y;
}

void TransformPositionXY3D(Vec3& posToTransform, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translateXY)
{
    Vec2 temp = Vec2(posToTransform.x, posToTransform.y);
    TransformPosition2D(temp, iBasis, jBasis, translateXY);
    posToTransform.x = temp.x;
    posToTransform.y = temp.y;
}

RaycastResult2D RaycastVsDisc2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& discCenter, float discRadius)
{
    if (IsPointInsideDisc2D(startPos, discCenter, discRadius))
    {
        return RaycastResult2D(0.0f, startPos, -fwdNormal);
    }

    float distToNearest = DotProduct2D(discCenter - startPos, fwdNormal);
    if (distToNearest < 0)
    {
        return RaycastResult2D();
    }

    Vec2 nearest = startPos + fwdNormal * distToNearest;
    float distSqNearestToDisc = GetDistanceSquared2D(nearest, discCenter);
    float discRadiusSq = discRadius * discRadius;
    if (distSqNearestToDisc < discRadiusSq)
    {
        // nearest in disc

        float distSqNearestToImpact = discRadiusSq - distSqNearestToDisc;
        float distNearestToImpact = sqrtf(distSqNearestToImpact);
        float impactDist = distToNearest - distNearestToImpact;

        if (impactDist < maxDist)
        {
            // impact position in ray

            Vec2 impactPos = startPos + fwdNormal * impactDist;
            Vec2 impactNormal = (impactPos - discCenter).GetNormalized();
            return RaycastResult2D(impactDist, impactPos, impactNormal);
        }
    }

    // not impact
    return RaycastResult2D();
}

RaycastResult2D RaycastVsAABB2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const AABB2& box)
{
    if (box.IsPointInside(startPos))
    {
        return RaycastResult2D(0.0f, startPos, -fwdNormal);
    }

    FloatRange xRange = FloatRange(box.m_mins.x, box.m_maxs.x);
    FloatRange yRange = FloatRange(box.m_mins.y, box.m_maxs.y);

    float impactDistMinX = (box.m_mins.x - startPos.x) / fwdNormal.x;
    float impactDistMaxX = (box.m_maxs.x - startPos.x) / fwdNormal.x;
	float impactDistMinY = (box.m_mins.y - startPos.y) / fwdNormal.y;
	float impactDistMaxY = (box.m_maxs.y - startPos.y) / fwdNormal.y;

    if (impactDistMinX < 0 || !yRange.IsOnRange((startPos + fwdNormal * impactDistMinX).y))
		impactDistMinX = maxDist + 1.0f;
	if (impactDistMaxX < 0 || !yRange.IsOnRange((startPos + fwdNormal * impactDistMaxX).y))
		impactDistMaxX = maxDist + 1.0f;
	if (impactDistMinY < 0 || !xRange.IsOnRange((startPos + fwdNormal * impactDistMinY).x))
		impactDistMinY = maxDist + 1.0f;
	if (impactDistMaxY < 0 || !xRange.IsOnRange((startPos + fwdNormal * impactDistMaxY).x))
		impactDistMaxY = maxDist + 1.0f;

    float nearest = impactDistMinX;
    Vec2 impactNormal = Vec2(-1.0f, 0.0f);
	if (impactDistMaxX < nearest)
	{
		nearest = impactDistMaxX;
		impactNormal = Vec2(1.0f, 0.0f);
	}
	if (impactDistMinY < nearest)
	{
		nearest = impactDistMinY;
		impactNormal = Vec2(0.0f, -1.0f);
	}
	if (impactDistMaxY < nearest)
	{
		nearest = impactDistMaxY;
		impactNormal = Vec2(0.0f, 1.0f);
	}

    if (nearest < maxDist)
    {
        return RaycastResult2D(nearest, startPos + fwdNormal * nearest, impactNormal);
    }
    else
    {
        return RaycastResult2D();
    }
}

RaycastResult2D RaycastVsOBB2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const OBB2& box)
{
	if (box.IsPointInside(startPos))
	{
		return RaycastResult2D(0.0f, startPos, -fwdNormal);
	}

    Vec2 startPosLocal  = box.GetLocalPosForWorldPos(startPos);
    Vec2 fwdNormalLocal = box.GetLocalPosForWorldPos(box.m_centerPos + fwdNormal);

	FloatRange xRangeLocal = FloatRange(-box.m_halfSize.x, box.m_halfSize.x);
	FloatRange yRangeLocal = FloatRange(-box.m_halfSize.y, box.m_halfSize.y);

	float impactDistMinX = (-box.m_halfSize.x - startPosLocal.x) / fwdNormalLocal.x;
	float impactDistMaxX = ( box.m_halfSize.x - startPosLocal.x) / fwdNormalLocal.x;
	float impactDistMinY = (-box.m_halfSize.y - startPosLocal.y) / fwdNormalLocal.y;
	float impactDistMaxY = ( box.m_halfSize.y - startPosLocal.y) / fwdNormalLocal.y;

	if (impactDistMinX < 0 || !yRangeLocal.IsOnRange((startPosLocal + fwdNormalLocal * impactDistMinX).y))
		impactDistMinX = maxDist + 1.0f;
	if (impactDistMaxX < 0 || !yRangeLocal.IsOnRange((startPosLocal + fwdNormalLocal * impactDistMaxX).y))
		impactDistMaxX = maxDist + 1.0f;
	if (impactDistMinY < 0 || !xRangeLocal.IsOnRange((startPosLocal + fwdNormalLocal * impactDistMinY).x))
		impactDistMinY = maxDist + 1.0f;
	if (impactDistMaxY < 0 || !xRangeLocal.IsOnRange((startPosLocal + fwdNormalLocal * impactDistMaxY).x))
		impactDistMaxY = maxDist + 1.0f;

	float nearest = impactDistMinX;
	Vec2 impactNormal = Vec2(-1.0f, 0.0f);
	if (impactDistMaxX < nearest)
	{
		nearest = impactDistMaxX;
		impactNormal = Vec2(1.0f, 0.0f);
	}
	if (impactDistMinY < nearest)
	{
		nearest = impactDistMinY;
		impactNormal = Vec2(0.0f, -1.0f);
	}
	if (impactDistMaxY < nearest)
	{
		nearest = impactDistMaxY;
		impactNormal = Vec2(0.0f, 1.0f);
	}

	if (nearest < maxDist)
	{
		return RaycastResult2D(nearest, startPos + fwdNormal * nearest, box.GetWorldPosForLocalPos(impactNormal) - box.m_centerPos);
	}
	else
	{
		return RaycastResult2D();
	}
}

RaycastResult2D RaycastVsLineSegment2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& lineStart, const Vec2& lineEnd)
{
    const Vec2& iNormal = fwdNormal;
	Vec2 jNormal = iNormal.GetRotated90Degrees();
    Vec2 RS = lineStart - startPos;
    Vec2 RE = lineEnd - startPos;
    float iprojRS = RS.Dot(iNormal);
    float iprojRE = RE.Dot(iNormal);
	float jprojRS = RS.Dot(jNormal);
	float jprojRE = RE.Dot(jNormal);

    if ((jprojRS > 0) == (jprojRE > 0) || (jprojRS - jprojRE) == 0)
        return RaycastResult2D();

    float impDist = (iprojRE * jprojRS - iprojRS * jprojRE) / (jprojRS - jprojRE);
    if (impDist >= maxDist || impDist < 0)
        return RaycastResult2D();

    Vec2 lineVec = lineEnd - lineStart;
    Vec2 impNormal = lineVec.GetNormalized().GetRotated90Degrees();
    if (jprojRS > 0)
        impNormal *= -1.0f;

    return RaycastResult2D(impDist, startPos + impDist * fwdNormal, impNormal);
}

RaycastResult2D RaycastVsPlane2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& planeDir, float planeDist)
{
	return Plane2D(planeDir, planeDist).Raycast(startPos, fwdNormal, maxDist);
}

RaycastResult3D RaycastVsAABB3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const AABB3& box)
{
	if (box.IsPointInside(startPos))
	{
		return RaycastResult3D(0.0f, startPos, -fwdNormal);
	}

	FloatRange xRange = FloatRange(box.m_mins.x, box.m_maxs.x);
	FloatRange yRange = FloatRange(box.m_mins.y, box.m_maxs.y);
	FloatRange zRange = FloatRange(box.m_mins.z, box.m_maxs.z);

	float impactDistMinX = (box.m_mins.x - startPos.x) / fwdNormal.x;
	float impactDistMaxX = (box.m_maxs.x - startPos.x) / fwdNormal.x;
	float impactDistMinY = (box.m_mins.y - startPos.y) / fwdNormal.y;
	float impactDistMaxY = (box.m_maxs.y - startPos.y) / fwdNormal.y;
	float impactDistMinZ = (box.m_mins.z - startPos.z) / fwdNormal.z;
	float impactDistMaxZ = (box.m_maxs.z - startPos.z) / fwdNormal.z;

	Vec3 impactPosMinX = startPos + fwdNormal * impactDistMinX;
	Vec3 impactPosMaxX = startPos + fwdNormal * impactDistMaxX;
	Vec3 impactPosMinY = startPos + fwdNormal * impactDistMinY;
	Vec3 impactPosMaxY = startPos + fwdNormal * impactDistMaxY;
	Vec3 impactPosMinZ = startPos + fwdNormal * impactDistMinZ;
	Vec3 impactPosMaxZ = startPos + fwdNormal * impactDistMaxZ;

	if (impactDistMinX < 0 || !yRange.IsOnRange(impactPosMinX.y) || !zRange.IsOnRange(impactPosMinX.z))
		impactDistMinX = maxDist + 1.0f;
	if (impactDistMaxX < 0 || !yRange.IsOnRange(impactPosMaxX.y) || !zRange.IsOnRange(impactPosMaxX.z))
		impactDistMaxX = maxDist + 1.0f;
	if (impactDistMinY < 0 || !xRange.IsOnRange(impactPosMinY.x) || !zRange.IsOnRange(impactPosMinY.z))
		impactDistMinY = maxDist + 1.0f;
	if (impactDistMaxY < 0 || !xRange.IsOnRange(impactPosMaxY.x) || !zRange.IsOnRange(impactPosMaxY.z))
		impactDistMaxY = maxDist + 1.0f;
	if (impactDistMinZ < 0 || !xRange.IsOnRange(impactPosMinZ.x) || !yRange.IsOnRange(impactPosMinZ.y))
		impactDistMinZ = maxDist + 1.0f;
	if (impactDistMaxZ < 0 || !xRange.IsOnRange(impactPosMaxZ.x) || !yRange.IsOnRange(impactPosMaxZ.y))
		impactDistMaxZ = maxDist + 1.0f;

	float nearest = impactDistMinX;
	Vec3 impactNormal = Vec3(-1.0f, 0.0f, 0.0f);
	if (impactDistMaxX < nearest)
	{
		nearest = impactDistMaxX;
		impactNormal = Vec3(1.0f, 0.0f, 0.0f);
	}
	if (impactDistMinY < nearest)
	{
		nearest = impactDistMinY;
		impactNormal = Vec3(0.0f, -1.0f, 0.0f);
	}
	if (impactDistMaxY < nearest)
	{
		nearest = impactDistMaxY;
		impactNormal = Vec3(0.0f, 1.0f, 0.0f);
	}
	if (impactDistMinZ < nearest)
	{
		nearest = impactDistMinZ;
		impactNormal = Vec3(0.0f, 0.0f, -1.0f);
	}
	if (impactDistMaxZ < nearest)
	{
		nearest = impactDistMaxZ;
		impactNormal = Vec3(0.0f, 0.0f, 1.0f);
	}

	if (nearest < maxDist)
	{
		return RaycastResult3D(nearest, startPos + fwdNormal * nearest, impactNormal);
	}
	else
	{
		return RaycastResult3D();
	}
}

RaycastResult3D RaycastVsSphere(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const Vec3& sphereCenter, float sphereRadius)
{
	if (IsPointInsideSphere(startPos, sphereCenter, sphereRadius))
	{
		return RaycastResult3D(0.0f, startPos, -fwdNormal);
	}

	float distToNearest = DotProduct3D(sphereCenter - startPos, fwdNormal);
	if (distToNearest < 0)
	{
		return RaycastResult3D();
	}

	Vec3 nearest = startPos + fwdNormal * distToNearest;
	float distSqNearestToSphere = GetDistanceSquared3D(nearest, sphereCenter);
	float sphereRadiusSq = sphereRadius * sphereRadius;
	if (distSqNearestToSphere < sphereRadiusSq)
	{
		// nearest in sphere

		float distSqNearestToImpact = sphereRadiusSq - distSqNearestToSphere;
		float distNearestToImpact = sqrtf(distSqNearestToImpact);
		float impactDist = distToNearest - distNearestToImpact;

		if (impactDist < maxDist)
		{
			// impact position in ray

			Vec3 impactPos = startPos + fwdNormal * impactDist;
			Vec3 impactNormal = (impactPos - sphereCenter).GetNormalized();
			return RaycastResult3D(impactDist, impactPos, impactNormal);
		}
	}

	// not impact
	return RaycastResult3D();
}

RaycastResult3D RaycastVsZCylinder(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight)
{
    Vec2 startPosXY = Vec2(startPos.x, startPos.y);
	Vec2 fwdNormalXY = Vec2(fwdNormal.x, fwdNormal.y);
	float fwdLengthXY = fwdNormalXY.NormalizeAndGetPreviousLength();
    float maxDistXY = fwdLengthXY * maxDist;
    Vec2 cylinderCenterXY = Vec2(cylinderBottomCenter.x, cylinderBottomCenter.y);

    RaycastResult2D impactDisc2D = RaycastVsDisc2D(startPosXY, fwdNormalXY, maxDistXY, cylinderCenterXY, cylinderRadius);
    if (!impactDisc2D.DidImpact())
    {
        return RaycastResult3D();
    }

    float impactDiscDist = impactDisc2D.GetImpactDistance() * (1.0f / fwdLengthXY);
    Vec3 impactDiscPos = startPos + fwdNormal * impactDiscDist;

    if (impactDiscPos.z < cylinderBottomCenter.z + cylinderHeight && impactDiscPos.z > cylinderBottomCenter.z)
    {
        Vec3 impactNormal = IsPointInsideDisc2D(startPosXY, cylinderCenterXY, cylinderRadius) ? -fwdNormal : Vec3(impactDisc2D.GetImpactNormal());
        return RaycastResult3D(impactDiscDist, impactDiscPos, impactNormal);
    }

    if (fwdNormal.z > 0)
    {
        float planeZ = cylinderBottomCenter.z;
        if (startPos.z <= planeZ)
        {
            float impactDist = (planeZ - startPos.z) / fwdNormal.z;
            if (impactDist < maxDist)
            {
                Vec3 impactPos = startPos + impactDist * fwdNormal;
                if ((cylinderBottomCenter - impactPos).GetLengthXYSquared() < cylinderRadius * cylinderRadius)
                {
                    return RaycastResult3D(impactDist, impactPos, Vec3(0.0f, 0.0f, -1.0f));
                }
            }
        }
    }
    else
	{
		float planeZ = cylinderBottomCenter.z + cylinderHeight;
		if (startPos.z >= planeZ)
		{
			float impactDist = (planeZ - startPos.z) / fwdNormal.z;
			if (impactDist < maxDist)
			{
				Vec3 impactPos = startPos + impactDist * fwdNormal;
				if ((cylinderBottomCenter - impactPos).GetLengthXYSquared() < cylinderRadius * cylinderRadius)
				{
					return RaycastResult3D(impactDist, impactPos, Vec3(0.0f, 0.0f, 1.0f));
				}
			}
		}
    }

    return RaycastResult3D();
}

EulerAngles DirectionToRotation(Vec3 direction)
{
	EulerAngles rot;
	const float xCoord = -direction.y;
	const float yCoord = direction.z;
	const float zCoord = direction.x;

	constexpr float epsilon = 1.192092896e-07f;
	constexpr float _2PI = 2 * PI;

	if (fabsf(xCoord) < epsilon && fabsf(zCoord) < epsilon) {
		rot.m_pitchDegrees = yCoord > 0.0f ? -90.0f : 90.0f;
		return rot;
	}

	float theta = atan2f(-xCoord, zCoord);
	rot.m_yawDegrees = ConvertRadiansToDegrees(fmodf(theta + _2PI, _2PI));

	float xz2 = sqrtf(xCoord * xCoord + zCoord * zCoord);
	rot.m_pitchDegrees = ConvertRadiansToDegrees(atanf(-yCoord / xz2));

	return rot;
}

bool IsPointInsideTriangle2D(const Vec2& point, const Vec2& pos0, const Vec2& pos1, const Vec2& pos2)
{
    float x0 = (point - pos0).Cross(pos1 - pos0);
    float x1 = (point - pos1).Cross(pos2 - pos1);
    if (x0 > 0 != x1 > 0)
        return false;
	float x2 = (point - pos2).Cross(pos0 - pos2);
	if (x0 > 0 != x2 > 0)
		return false;
    return true;
}

bool IsPointInsideConvexPolygon2D(const Vec2& point, int polygonNum, const Vec2* polygonPositions)
{
    float x0 = (point - polygonPositions[polygonNum - 1]).Cross(polygonPositions[0] - polygonPositions[polygonNum - 1]);
    for (int i = 0; i < polygonNum - 1; i++)
    {
        float x1 = (point - polygonPositions[i]).Cross(polygonPositions[i + 1] - polygonPositions[i]);
		if (x0 > 0 != x1 > 0)
			return false;
    }
    return true;
}

void CalcBarycentric(const Vec3& point, const Vec3& a, const Vec3& b, const Vec3& c, float& out_u, float& out_v, float& out_w)
{
    float xTriangleInv = 1.0f / (b - a).Cross2D(c - a);

	float xPB = (c - b).Cross2D(point - b);
	float xPC = (a - c).Cross2D(point - c);

    out_u = xPB * xTriangleInv;
    out_v = xPC * xTriangleInv;
    out_w = 1.0f - out_u - out_v;
}

