#include "AABB3.hpp"

#include "MathUtils.hpp"

const AABB3 AABB3::ZERO_TO_ONE = AABB3(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

AABB3::AABB3(const AABB3& copyFrom)
	: m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}

AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	: m_mins(Vec3(minX, minY, minZ))
	, m_maxs(Vec3(maxX, maxY, maxZ))
{
}

AABB3::AABB3(const Vec3& mins, const Vec3& maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}

AABB3::AABB3(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
	: m_mins(Vec3((float)minX, (float)minY, (float)minZ))
	, m_maxs(Vec3((float)maxX, (float)maxY, (float)maxZ))
{

}

bool AABB3::IsPointInside(const Vec3& point) const
{
	return 
		   point.x >= m_mins.x && point.x <= m_maxs.x
		&& point.y >= m_mins.y && point.y <= m_maxs.y
		&& point.z >= m_mins.z && point.z <= m_maxs.z;
}

Vec3 AABB3::GetCenter() const
{
	return GetPointAtUV(Vec3(0.5f, 0.5f, 0.5f));
}

Vec3 AABB3::GetDimensions() const
{
	return m_maxs - m_mins;
}

Vec3 AABB3::GetNearestPoint(const Vec3& point) const
{
	return Vec3(Clamp(point.x, m_mins.x, m_maxs.x), Clamp(point.y, m_mins.y, m_maxs.y), Clamp(point.z, m_mins.z, m_maxs.z));
}

Vec3 AABB3::GetPointAtUV(const Vec3& uv) const
{
	return m_mins + GetDimensions() * uv;
}

Vec3 AABB3::GetUVForPoint(const Vec3& point) const
{
	float u = GetFractionWithin(point.x, m_mins.x, m_maxs.x);
	float v = GetFractionWithin(point.y, m_mins.y, m_maxs.y);
	float w = GetFractionWithin(point.z, m_mins.z, m_maxs.z);
	return Vec3(u, v, w);
}

void AABB3::Translate(const Vec3& translation)
{
	m_mins += translation;
	m_maxs += translation;
}

void AABB3::SetCenter(const Vec3& newCenter)
{
	Translate(newCenter - GetCenter());
}

void AABB3::SetDimensions(const Vec3& newDimensions)
{
	Vec3 center = GetCenter();
	m_mins = center - newDimensions * 0.5f;
	m_maxs = center + newDimensions * 0.5f;
}

void AABB3::StretchToIncludePoint(const Vec3& point)
{
	m_mins.x = Min(m_mins.x, point.x);
	m_mins.y = Min(m_mins.y, point.y);
	m_mins.z = Min(m_mins.z, point.z);
	m_maxs.x = Max(m_maxs.x, point.x);
	m_maxs.y = Max(m_maxs.y, point.y);
	m_maxs.z = Max(m_maxs.z, point.z);
}

void AABB3::AlignToBox(const AABB3& target, const Vec3& alignment)
{
	Vec3 dims = GetDimensions();
	Vec3 dimsTarget = target.GetDimensions();
	Vec3 dimsDiff = dimsTarget - dims;

	SetCenter(target.GetCenter());
	Translate(dimsDiff * (alignment - Vec3(0.5f, 0.5f, 0.5f)));
}

void AABB3::AlignToBoxHorizontal(const AABB3& target, float alignment)
{
	float dimsX = m_maxs.x - m_mins.x;
	float dimsTargetX = target.m_maxs.x - target.m_mins.x;
	float dimsDiffX = dimsTargetX - dimsX;

	float xOffset = Lerp(target.m_mins.x, target.m_maxs.x, 0.5f) - Lerp(m_mins.x, m_maxs.x, 0.5f);
	m_mins.x += xOffset;
	m_maxs.x += xOffset;

	float translateX = dimsDiffX * (alignment - 0.5f);
	m_mins.x += translateX;
	m_maxs.x += translateX;
}

void AABB3::AlignToBoxVertical(const AABB3& target, float alignment)
{
	float dimsY = m_maxs.y - m_mins.y;
	float dimsTargetY = target.m_maxs.y - target.m_mins.y;
	float dimsDiffY = dimsTargetY - dimsY;

	float yOffset = Lerp(target.m_mins.y, target.m_maxs.y, 0.5f) - Lerp(m_mins.y, m_maxs.y, 0.5f);
	m_mins.y += yOffset;
	m_maxs.y += yOffset;

	float translateY = dimsDiffY * (alignment - 0.5f);
	m_mins.y += translateY;
	m_maxs.y += translateY;
}

AABB3 AABB3::GetShifted(const Vec3& offset) const
{
	AABB3 shifted = *this;
	shifted.SetCenter(shifted.GetCenter() + offset);
	return shifted;
}

