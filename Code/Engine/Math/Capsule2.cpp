#include "Capsule2.hpp"

#include "MathUtils.hpp"

Capsule2::Capsule2(const Vec2& boneStartPos, const Vec2& boneEndPos, float radius)
	: m_bone(LineSegment2(boneStartPos, boneEndPos))
	, m_radius(radius)
{
}

Capsule2::Capsule2(const LineSegment2& bone, float radius)
	: m_bone(bone)
	, m_radius(radius)
{
}

const Vec2 Capsule2::GetNearestPoint(const Vec2& point) const
{
	Vec2 nearest = m_bone.GetNearestPoint(point);
	Vec2 direction = point - nearest;
	direction.SetLength(m_radius);
	return nearest + direction;
}

bool Capsule2::IsPointInside(const Vec2& point) const
{
	Vec2 nearest = m_bone.GetNearestPoint(point);
	return (point - nearest).GetLengthSquared() < m_radius * m_radius;
}

const Vec2 Capsule2::GetCenter() const
{
	return m_bone.GetCenter();
}

void Capsule2::Translate(const Vec2& translation)
{
	m_bone.Translate(translation);
}

void Capsule2::SetCenter(const Vec2& newCenter)
{
	m_bone.SetCenter(newCenter);
}

void Capsule2::RotateAboutCenter(float rotationDeltaDegrees)
{
	m_bone.RotateAboutCenter(rotationDeltaDegrees);
}

bool Capsule2::operator==(const Capsule2& compare) const
{
	return m_bone == compare.m_bone && m_radius == compare.m_radius;
}

bool Capsule2::operator!=(const Capsule2& compare) const
{
	return !operator==(compare);
}

void Capsule2::operator=(const Capsule2& copyFrom)
{
	m_bone = copyFrom.m_bone;
	m_radius = copyFrom.m_radius;
}

