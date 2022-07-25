#pragma once

#include "Vec2.hpp"

struct LineSegment2
{
public:
	Vec2 m_startPos;
	Vec2 m_endPos;

public:
	~LineSegment2() {}
	LineSegment2() {}
	LineSegment2(const LineSegment2& copyFrom);
	explicit LineSegment2(const Vec2& startPos, const Vec2& endPos);

	// utilities
	const Vec2        GetNearestPoint(const Vec2& point) const;
	const Vec2        GetNearestPointOnInfiniteLine(const Vec2& point) const;
	const Vec2        GetSlopeVectorNormalized() const;
	float             GetGradientDegrees() const;
	const Vec2        GetCenter() const;
	float             GetLength() const;
	void              Translate(const Vec2& translation);
	void              SetCenter(const Vec2& newCenter);
	void              RotateAboutCenter(float rotationDeltaDegrees);

	// Operators
	bool              operator==(const LineSegment2& compare) const;
	bool              operator!=(const LineSegment2& compare) const;
	void              operator= (const LineSegment2& copyFrom);
};

