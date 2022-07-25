#include "PolarPosition2D.hpp"

#include "Vec2.hpp"

PolarPosition2D::~PolarPosition2D()
{

}

PolarPosition2D::PolarPosition2D()
	: radius(0)
	, angleDegrees(0)
{
}

PolarPosition2D::PolarPosition2D(float p_radius, float p_angleDegrees)
	: radius(p_radius)
	, angleDegrees(p_angleDegrees)
{
}

PolarPosition2D::PolarPosition2D(const Vec2& fromVec2)
{
	radius = fromVec2.GetLength();
	angleDegrees = fromVec2.GetOrientationDegrees();
}

Vec2 PolarPosition2D::ConvertToVec2() const
{
	Vec2 vec2 = Vec2();
	if (radius == 0)
	{
		return vec2;
	}
	vec2.SetPolarDegrees(angleDegrees, radius);
	return vec2;
}

