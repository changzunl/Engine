#include "OBB2.hpp"

#include "AABB2.hpp"
#include "MathUtils.hpp"

OBB2::OBB2(const Vec2& rightDir, const Vec2& centerPos, const Vec2& bounds)
	: m_rightDir(rightDir)
	, m_centerPos(centerPos)
	, m_halfSize(bounds)
{
}

OBB2::OBB2(float orientationDegrees, const Vec2& centerPos, const Vec2& bounds)
	: m_rightDir(Vec2::MakeFromPolarDegrees(orientationDegrees))
	, m_centerPos(centerPos)
	, m_halfSize(bounds)
{
}

bool OBB2::IsPointInside(const Vec2& point) const
{
	Vec2 localPoint = GetLocalPosForWorldPos(point);
	AABB2 localBox = AABB2(-m_halfSize, m_halfSize);
	return localBox.IsPointInside(localPoint);
}

Vec2 OBB2::GetNearestPoint(const Vec2& point) const
{
	Vec2 localPoint = GetLocalPosForWorldPos(point);
	AABB2 localBox = AABB2(-m_halfSize, m_halfSize);
	Vec2 localNearest = localBox.GetNearestPoint(localPoint);
	return GetWorldPosForLocalPos(localNearest);
}

void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	Vec2 ibasis = m_rightDir;
	Vec2 jbasis = ibasis.GetRotated90Degrees();

	Vec2 point0 = -m_halfSize.x * ibasis +  m_halfSize.y * jbasis;
	Vec2 point1 = -m_halfSize.x * ibasis + -m_halfSize.y * jbasis;
	Vec2 point2 = -point1;
	Vec2 point3 = -point0;

	out_fourCornerWorldPositions[0] = point0 + m_centerPos;
	out_fourCornerWorldPositions[1] = point1 + m_centerPos;
	out_fourCornerWorldPositions[2] = point2 + m_centerPos;
	out_fourCornerWorldPositions[3] = point3 + m_centerPos;
}

Vec2 OBB2::GetLocalPosForWorldPos(const Vec2& worldPos) const
{
	Vec2 ibasis = m_rightDir;
	Vec2 jbasis = ibasis.GetRotated90Degrees();

	Vec2 relativePos = worldPos - m_centerPos;
	return Vec2(relativePos.Dot(ibasis), relativePos.Dot(jbasis));
}

Vec2 OBB2::GetWorldPosForLocalPos(const Vec2& localPos) const
{
	Vec2 ibasis = m_rightDir;
	Vec2 jbasis = ibasis.GetRotated90Degrees();

	Vec2 worldPos = localPos;
	TransformPosition2D(worldPos, ibasis, jbasis, m_centerPos);
	return worldPos;
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_rightDir.RotateDegrees(rotationDeltaDegrees);
}

bool OBB2::operator==(const OBB2& compare) const
{
	return m_centerPos == compare.m_centerPos && m_rightDir == compare.m_rightDir && m_halfSize == compare.m_halfSize;
}

bool OBB2::operator!=(const OBB2& compare) const
{
	return !operator==(compare);
}

void OBB2::operator=(const OBB2& copyFrom)
{
	m_centerPos = copyFrom.m_centerPos;
	m_rightDir  = copyFrom.m_rightDir;
	m_halfSize  = copyFrom.m_halfSize;
}

