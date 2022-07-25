#pragma once

struct Vec2;

struct PolarPosition2D
{
public:
	float radius          = 0.0f;
	float angleDegrees    = 0.0f;

public:
	~PolarPosition2D();
	PolarPosition2D();
	PolarPosition2D(const float p_radius, const float p_angleDegrees);
	PolarPosition2D(const Vec2& fromVec2);
	Vec2 ConvertToVec2() const;
};

