#pragma once

#include "Vec2.hpp"

struct OBB2
{
public:
	Vec2 m_centerPos = Vec2::ZERO;
	Vec2 m_halfSize = Vec2::ZERO;

public:
	~OBB2() {};
	OBB2() {};
	OBB2(const Vec2& rightDir, const Vec2& centerPos, const Vec2& bounds);
	OBB2(float orientationDegrees, const Vec2& centerPos, const Vec2& bounds);

	// utilities
	Vec2 GetRightDirection() const                                   { return m_rightDir; }
	bool IsPointInside(const Vec2& point) const;
	Vec2 GetNearestPoint(const Vec2& point) const;
	void GetCornerPoints(Vec2* out_fourCornerWorldPositions) const; // for drawing, etc.!
	Vec2 GetLocalPosForWorldPos(const Vec2& worldPos) const;
	Vec2 GetWorldPosForLocalPos(const Vec2& localPos) const;
	void RotateAboutCenter(float rotationDeltaDegrees);

	// Operators
	bool operator==(const OBB2& compare) const;
	bool operator!=(const OBB2& compare) const;
	void operator=(const OBB2& copyFrom);

private:
	Vec2 m_rightDir = Vec2::ONE;
};

