#include "LineSegment2.hpp"

#include "MathUtils.hpp"

LineSegment2::LineSegment2(const LineSegment2& copyFrom)
	: m_startPos(copyFrom.m_startPos)
	, m_endPos(copyFrom.m_endPos)
{
}

LineSegment2::LineSegment2(const Vec2& startPos, const Vec2& endPos)
	: m_startPos(startPos)
	, m_endPos(endPos)
{
}

const Vec2 LineSegment2::GetNearestPoint(const Vec2& point) const
{
	Vec2 lineVector = m_endPos - m_startPos;
	if (DotProduct2D(point - m_startPos, lineVector) <= 0) return m_startPos;

	if (DotProduct2D(point - m_endPos, -lineVector) <= 0) return m_endPos;

	Vec2 lineDirection = lineVector.GetNormalized();
	return m_startPos + lineDirection * DotProduct2D(point - m_startPos, lineDirection);
}

const Vec2 LineSegment2::GetNearestPointOnInfiniteLine(const Vec2& point) const
{
	Vec2 lineDirection = GetSlopeVectorNormalized();
	return m_startPos + lineDirection * DotProduct2D(point - m_startPos, lineDirection);
}

const Vec2 LineSegment2::GetSlopeVectorNormalized() const
{
	return (m_endPos - m_startPos).GetNormalized();
}

float LineSegment2::GetGradientDegrees() const
{
	return (m_endPos - m_startPos).GetOrientationDegrees();
}

const Vec2 LineSegment2::GetCenter() const
{
	return Vec2(Lerp(m_startPos.x, m_endPos.x, 0.5f), Lerp(m_startPos.y, m_endPos.y, 0.5f));
}

float LineSegment2::GetLength() const
{
	return (m_endPos - m_startPos).GetLength();
}

void LineSegment2::Translate(const Vec2& translation)
{
	m_startPos += translation;
	m_endPos   += translation;
}

void LineSegment2::SetCenter(const Vec2& newCenter)
{
	Translate(newCenter - GetCenter());
}

void LineSegment2::RotateAboutCenter(float rotationDeltaDegrees)
{
	Vec2 center = GetCenter();
	Vec2 forward = m_startPos - center;
	forward.RotateDegrees(rotationDeltaDegrees);
	m_startPos = center + forward;
	m_endPos = center - forward;
}

bool LineSegment2::operator==(const LineSegment2& compare) const
{
	return (m_startPos == compare.m_startPos && m_endPos == compare.m_endPos) ||
		(m_startPos == compare.m_endPos && m_endPos == compare.m_startPos);
}

bool LineSegment2::operator!=(const LineSegment2& compare) const
{
	return !operator==(compare);
}

void LineSegment2::operator=(const LineSegment2& copyFrom)
{
	m_startPos = copyFrom.m_startPos;
	m_endPos = copyFrom.m_endPos;
}

