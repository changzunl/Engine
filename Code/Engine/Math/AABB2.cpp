#include "AABB2.hpp"

#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

const AABB2 AABB2::ZERO_TO_ONE = AABB2(0.0f, 0.0f, 1.0f, 1.0f);

AABB2::AABB2(const AABB2& copyFrom)
	: m_mins(copyFrom.m_mins)
	, m_maxs(copyFrom.m_maxs)
{
}

AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: m_mins(Vec2(minX, minY))
	, m_maxs(Vec2(maxX, maxY))
{
}

AABB2::AABB2(const Vec2& mins, const Vec2& maxs)
	: m_mins(mins)
	, m_maxs(maxs)
{
}

AABB2::AABB2(AABB2&& moveFrom)
	: m_mins(moveFrom.m_mins)
	, m_maxs(moveFrom.m_maxs)
{
	moveFrom.SetDimensions(Vec2::ZERO);
}

bool AABB2::IsPointInside(const Vec2& point) const
{
	return point.x >= m_mins.x && point.x <= m_maxs.x && point.y >= m_mins.y && point.y <= m_maxs.y;
}

Vec2 AABB2::GetCenter() const
{
	return GetPointAtUV(Vec2(0.5f, 0.5f));
}

Vec2 AABB2::GetDimensions() const
{
	return m_maxs - m_mins;
}

Vec2 AABB2::GetNearestPoint(const Vec2& point) const
{
	return Vec2(Clamp(point.x, m_mins.x, m_maxs.x), Clamp(point.y, m_mins.y, m_maxs.y));
}

Vec2 AABB2::GetPointAtUV(const Vec2& uv) const
{
	return m_mins + GetDimensions() * uv;
}

Vec2 AABB2::GetUVForPoint(const Vec2& point) const
{
	float u = GetFractionWithin(point.x, m_mins.x, m_maxs.x);
	float v = GetFractionWithin(point.y, m_mins.y, m_maxs.y);
	return Vec2(u, v);
}

AABB2 AABB2::GetSubBox(const AABB2& uv) const
{
	return AABB2(GetPointAtUV(uv.m_mins), GetPointAtUV(uv.m_maxs));
}

FloatRange AABB2::GetRangeX() const
{
	return FloatRange(m_mins.x, m_maxs.x);
}

FloatRange AABB2::GetRangeY() const
{
    return FloatRange(m_mins.y, m_maxs.y);
}

AABB2 AABB2::GetIntersection(const AABB2& other) const
{
	Vec2 newMin(
		Max(m_mins.x, other.m_mins.x),
		Max(m_mins.y, other.m_mins.y)
	);
	Vec2 newMax(
		Min(m_maxs.x, other.m_maxs.x),
		Min(m_maxs.y, other.m_maxs.y)
	);
	return AABB2(newMin, newMax);
}

bool AABB2::HasIntersection(const AABB2& other) const
{
	// Check if the two AABBs are overlapping on the X axis
	if (m_maxs.x < other.m_mins.x || m_mins.x > other.m_maxs.x)
		return false;

	// Check if the two AABBs are overlapping on the Y axis
	if (m_maxs.y < other.m_mins.y || m_mins.y > other.m_maxs.y)
		return false;

	return true;
}

void AABB2::Translate(const Vec2& translation)
{
	m_mins += translation;
	m_maxs += translation;
}

void AABB2::SetCenter(const Vec2& newCenter)
{
	Translate(newCenter - GetCenter());
}

void AABB2::SetDimensions(const Vec2& newDimensions)
{
	Vec2 center = GetCenter();
	m_mins = center - newDimensions * 0.5f;
	m_maxs = center + newDimensions * 0.5f;
}

void AABB2::StretchToIncludePoint(const Vec2& point)
{
	m_mins.x = Min(m_mins.x, point.x);
	m_mins.y = Min(m_mins.y, point.y);
	m_maxs.x = Max(m_maxs.x, point.x);
	m_maxs.y = Max(m_maxs.y, point.y);
}

void AABB2::AlignToBox(const AABB2& target, const Vec2& alignment)
{
	Vec2 dims = GetDimensions();
	Vec2 dimsTarget = target.GetDimensions();
	Vec2 dimsDiff = dimsTarget - dims;

	SetCenter(target.GetCenter());
	Translate(dimsDiff * (alignment - Vec2(0.5f, 0.5f)));
}

void AABB2::AlignToBoxHorizontal(const AABB2& target, float alignment)
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

void AABB2::AlignToBoxVertical(const AABB2& target, float alignment)
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

AABB2 AABB2::ChopOffTop(float v, bool rel)
{
	float dim = GetDimensions().y;
	AABB2 chop = *this;
	m_maxs.y -= v * (rel ? dim : 1);
	chop.m_mins.y = m_maxs.y;
	return chop;
}

AABB2 AABB2::ChopOffBottom(float v, bool rel)
{
	float dim = GetDimensions().y;
	AABB2 chop = *this;
	m_mins.y += v * (rel ? dim : 1);
	chop.m_maxs.y = m_mins.y;
	return chop;
}

AABB2 AABB2::ChopOffLeft(float u, bool rel)
{
	float dim = GetDimensions().x;
	AABB2 chop = *this;
	m_mins.x += u * (rel ? dim : 1);
	chop.m_maxs.x = m_mins.x;
	return chop;
}

AABB2 AABB2::ChopOffRight(float u, bool rel)
{
	float dim = GetDimensions().x;
	AABB2 chop = *this;
	m_maxs.x -= u * (rel ? dim : 1);
	chop.m_mins.x = m_maxs.x;
	return chop;
}

void AABB2::operator=(AABB2&& other) noexcept
{
	m_mins = other.m_mins;
	m_maxs = other.m_maxs;
	other.SetDimensions(Vec2::ZERO);
}

void AABB2::operator=(const AABB2& other)
{
	m_mins = other.m_mins;
	m_maxs = other.m_maxs;
}

bool AABB2::operator!=(const AABB2& other)
{
	return !operator==(other);
}

bool AABB2::operator==(const AABB2& other)
{
	return m_mins == other.m_mins && m_maxs == other.m_maxs;
}

