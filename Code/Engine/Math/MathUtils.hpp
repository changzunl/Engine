#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/RaycastResult.hpp"

struct IntVec2;
struct Vec4;
struct AABB2;
struct AABB3;
struct OBB2;
struct LineSegment2;
struct Capsule2;
struct EulerAngles;

constexpr float  PI = 3.14159265358979323846f;

// Basic math utilities
template<typename T> 
T Min(T a, T b) { return b < a ? b : a; }
template<typename T>
T Max(T a, T b) { return b > a ? b : a; }
template<typename T>
T Clamp(T input, T lower, T upper) { return input < lower ? lower : (input > upper ? upper : input); }
void       ClampValue(float& value, float lower, float upper);
inline int MinInt(int a, int b) { return Min(a, b); }
inline int MaxInt(int a, int b) { return Max(a, b); }
inline int ClampInt(int input, int lower, int upper) { return Clamp(input, lower, upper); }
void       ClampIntValue(int& value, int lower, int upper);
float      ClampZeroToOne(float input);
void       ClampValueZeroToOne(float& value);
float      SquareRoot(float input);
int        Floor(float input);
int        Ceil(float input);
float      Lerp(float start, float end, float fraction);
float      GetFractionWithin(float value, float start, float end);
float      RangeMap(float rawValue, float inRangeStart, float inRangeEnd, float outRangeStart = 0.0f, float outRangeEnd = 1.0f);
float      RangeMapClamped(float rawValue, float inRangeStart, float inRangeEnd, float outRangeStart = 0.0f, float outRangeEnd = 1.0f);

float         NormalizeByte(unsigned char byteValue);
unsigned char DenormalizeByte(float floatValue);


// Basic triangular functions
float      ConvertDegreesToRadians(float degrees);
float      ConvertRadiansToDegrees(float radians);
float      CosDegrees(float degrees);
float      SinDegrees(float degrees);
float      Atan2Degrees(float y, float x);


// Basic 2D & 3D utilities
float      GetDistance2D(const Vec2& posA, const Vec2& posB);
float      GetDistanceSquared2D(const Vec2& posA, const Vec2& posB);
float      GetDistance3D(const Vec3& posA, const Vec3& posB);
float      GetDistanceSquared3D(const Vec3& posA, const Vec3& posB);
float      GetDistanceXY3D(const Vec3& posA, const Vec3& posB);
float      GetDistanceXYSquared3D(const Vec3& posA, const Vec3& posB);
int        GetTaxicabDistance2D(const IntVec2& pointA, const IntVec2& pointB);
float      DotProduct2D(const Vec2& pointA, const Vec2& pointB);
float      DotProduct3D(const Vec3& pointA, const Vec3& pointB);
float      DotProduct4D(const Vec4& pointA, const Vec4& pointB);
float      CrossProduct2D(const Vec2& pointA, const Vec2& pointB);
const Vec3 CrossProduct3D(const Vec3& pointA, const Vec3& pointB);
float      GetProjectedLength2D(const Vec2& vectorToProject, const Vec2& vectorToProjectOnto);
float      GetProjectedLength3D(const Vec3& vectorToProject, const Vec3& vectorToProjectOnto);
const Vec2 GetProjectedOnto2D(const Vec2& vectorToProject, const Vec2& vectorToProjectOnto);
const Vec3 GetProjectedOnto3D(const Vec3& vectorToProject, const Vec3& vectorToProjectOnto);
float      GetAngleDegreesBetweenVectors2D(const Vec2& pointA, const Vec2& pointB);

EulerAngles DirectionToRotation(Vec3 direction);
void        CalcBarycentric(const Vec3& point, const Vec3& a, const Vec3& b, const Vec3& c, float& out_u, float& out_v, float& out_w);

// Geometric query utilities
bool       IsPointInsideTriangle2D(const Vec2& point, const Vec2& pos0, const Vec2& pos1, const Vec2& pos2);
bool       IsPointInsideConvexPolygon2D(const Vec2& point, int polygonNum, const Vec2* polygonPositions);
bool       IsPointInsideDisc2D(const Vec2& point, const Vec2& discCenter, float discRadius);
bool       IsPointInsideAABB2D(const Vec2& point, const AABB2& aabb);
bool       IsPointInsideOBB2D(const Vec2& point, const OBB2& obb);
bool       IsPointInsideCapsule2D(const Vec2& point, const Capsule2& capsule);
bool       IsPointInsideCapsule2D(const Vec2& point, const Vec2& boneStartPos, const Vec2& boneEndPos, float radius);
bool       IsPointInsideOrientedSector2D(const Vec2& point, const Vec2& sectorTip, float sectorForwardDegrees, float sectorApertureDegrees, float sectorRadius);
bool       IsPointInsideDirectedSector2D(const Vec2& point, const Vec2& sectorTip, const Vec2& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);

bool       IsPointInsideSphere(const Vec3& point, const Vec3& sphereCenter, float sphereRadius);
bool       IsPointInsideAABB3D(const Vec3& point, const AABB3& aabb);
bool       IsPointInsideZCylinder(const Vec3& point, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);

bool       DoDiscsOverlap(const Vec2& centerA, float radiusA, const Vec2& centerB, float radiusB);
bool       DoAABB2Overlap(const AABB2& aabbA, const AABB2& aabbB);
bool       DoDiscOverlapAABB2(const Vec2& discCenter, float discRadius, const AABB2& aabb);
bool       DoDiscOverlapOBB2(const Vec2& discCenter, float discRadius, const OBB2& obb);
bool       DoDiscOverlapCapsule2(const Vec2& discCenter, float discRadius, const Capsule2& capsule);

bool       DoSpheresOverlap(const Vec3& posA, float radiusA, const Vec3& posB, float radiusB);
bool       DoAABB3Overlap(const AABB3& aabbA, const AABB3& aabbB);
bool       DoZCylinderOverlap(const Vec3& cylinderBottomCenterA, float cylinderRadiusA, float cylinderHeightA, const Vec3& cylinderBottomCenterB, float cylinderRadiusB, float cylinderHeightB);
bool       DoAABB3OverlapSphere(const AABB3& aabb, const Vec3& sphereCenter, float sphereRadius);
bool       DoZCylinderOverlapSphere(const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight, const Vec3& sphereCenter, float sphereRadius);
bool       DoAABB3OverlapZCylinder(const AABB3& aabb, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);

const Vec2 GetNearestPointOnDisc2D(const Vec2& referencePos, const Vec2& discCenter, float discRadius);
const Vec2 GetNearestPointOnInfiniteLine2D(const Vec2& referencePos, const LineSegment2& line);
const Vec2 GetNearestPointOnInfiniteLine2D(const Vec2& referencePos, const Vec2& pointAOnLine, const Vec2& pointBOnLine);
const Vec2 GetNearestPointOnLineSegmen2D(const Vec2& referencePos, const LineSegment2& lineSegment);
const Vec2 GetNearestPointOnLineSegmen2D(const Vec2& referencePos, const Vec2& pointAOnLine, const Vec2& pointBOnLine);
const Vec2 GetNearestPointOnAABB2D(const Vec2& referencePos, const AABB2& aabb);
const Vec2 GetNearestPointOnOBB2D(const Vec2& referencePos, const OBB2& obb);
const Vec2 GetNearestPointOnCapsule2D(const Vec2& referencePos, const Capsule2& capsule);
const Vec2 GetNearestPointOnCapsule2D(const Vec2& referencePos, const Vec2& boneStartPos, const Vec2& boneEndPos, float radius);

const Vec3 GetNearestPointOnSphere(const Vec3& referencePos, const Vec3& sphereCenter, float sphereRadius);
const Vec3 GetNearestPointOnAABB3D(const Vec3& referencePos, const AABB3& aabb);
const Vec3 GetNearestPointOnZCylinder(const Vec3& referencePos, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);

// physics correction
bool       PushDiscOutOfPoint2D(Vec2& mobileDiscCenter, float discRadius, const Vec2& fixedPoint);
bool       PushDiscOutOfDisc2D(Vec2& mobileDiscCenter, float mobileRadius, const Vec2& fixedDiscCenter, float fixedRadius);
bool       PushDiscsOutOfEachOther2D(Vec2& aCenter, float aRadius, Vec2& bCenter, float bRadius);
bool       PushDiscOutOfAABB2D(Vec2& mobileDiscCenter, float discRadius, const AABB2& fixedBox);
bool       PushDiscOutOfOBB2D(Vec2& mobileDiscCenter, float discRadius, const OBB2& fixedBox);
bool       BounceDiscOffEachOther(Vec2& pos1, float radius1, Vec2& vel1, float bounceness1, Vec2& pos2, float radius2, Vec2& vel2, float bounceness2);
bool       BounceDiscOffDisc(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileBounceness, const Vec2& fixedCenter, float fixedRadius, float fixedBounceness);
bool       BounceDiscOffOBB2(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileBounceness, const OBB2& fixedBox, float fixedBounceness);
bool       BounceDiscOffCapsule(Vec2& mobileCenter, float mobileRadius, Vec2& mobileVelocity, float mobileBounceness, const Capsule2& fixedCapsule, float fixedBounceness);
float      GetShortestAngularDispDegrees(float fromDegrees, float toDegrees);
float      GetTurnedTowardDegrees(float fromDegrees, float toDegrees, float maxTurnRateDegrees);

// Transform utilities
void       TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, const Vec2& translate);
void       TransformPosition2D(Vec2& posToTransform, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translate);
void       TransformPositionXY3D(Vec3& posToTransform, float uniformScale, float rotationDegrees, const Vec2& translateXY);
void       TransformPositionXY3D(Vec3& posToTransform, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translateXY);

// Raycast utilities
RaycastResult2D RaycastVsDisc2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& discCenter, float discRadius);
RaycastResult2D RaycastVsAABB2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const AABB2& box);
RaycastResult2D RaycastVsOBB2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const OBB2& box);
RaycastResult2D RaycastVsLineSegment2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& lineStart, const Vec2& lineEnd);
RaycastResult2D RaycastVsPlane2D(const Vec2& startPos, const Vec2& fwdNormal, float maxDist, const Vec2& planeDir, float planeDist);

RaycastResult3D RaycastVsSphere(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const Vec3& sphereCenter, float sphereRadius);
RaycastResult3D RaycastVsAABB3D(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const AABB3& box);
RaycastResult3D RaycastVsZCylinder(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, const Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight);

Vec2 Lerp(const Vec2& point1, const Vec2& point2, float fraction);
Vec3 Lerp(const Vec3& point1, const Vec3& point2, float fraction);

