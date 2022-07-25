#pragma once

#include "LineSegment2.hpp"

struct Capsule2
{
public:
	LineSegment2  m_bone;
	float m_radius          = 0.0f;

public:
	~Capsule2() {};
	Capsule2() {};
	Capsule2(const Vec2& boneStartPos, const Vec2& boneEndPos, float radius);
	Capsule2(const LineSegment2& line, float radius);

	// utilities
	const Vec2        GetNearestPoint(const Vec2& point) const;
	bool              IsPointInside(const Vec2& point) const;
	const Vec2        GetCenter() const;
	void              Translate(const Vec2& translation);
	void              SetCenter(const Vec2& newCenter);
	void              RotateAboutCenter(float rotationDeltaDegrees);

	// Operators
	bool              operator==(const Capsule2& compare) const;
	bool              operator!=(const Capsule2& compare) const;
	void              operator= (const Capsule2& copyFrom);
};

