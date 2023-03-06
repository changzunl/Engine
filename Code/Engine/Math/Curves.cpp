#include "Curves.hpp"


float SmoothStart2(float f)
{
    return f * f;
}
float SmoothStart3(float f)
{
	return f * f * f;
}

float SmoothStart4(float f)
{
	return f * f * f * f;
}

float SmoothStart5(float f)
{
	return f * f * f * f * f;
}

float SmoothStart6(float f)
{
	return f * f * f * f * f * f;
}

float SmoothStop2(float f)
{
	return 1.0f - SmoothStart2(1 - f);
}

float SmoothStop3(float f)
{
	return 1.0f - SmoothStart3(1 - f);
}

float SmoothStop4(float f)
{
	return 1.0f - SmoothStart4(1 - f);
}

float SmoothStop5(float f)
{
	return 1.0f - SmoothStart5(1 - f);
}

float SmoothStop6(float f)
{
	return 1.0f - SmoothStart6(1 - f);
}

float SmoothStep3(float f)
{
	float data[4] = { 0, 0, 1, 1 };
	return BezierCurveRecursive(4, &data[0], f);
}

float SmoothStep5(float f)
{
	float data[6] = { 0, 0, 0, 1, 1, 1 };
	return BezierCurveRecursive(6, &data[0], f);
}

float Hesitate3(float f)
{
	float data[4] = { 0, 1, 0, 1 };
	return BezierCurveRecursive(4, &data[0], f);
}

float Hesitate5(float f)
{
	float data[6] = { 0, 1, 0, 1, 0, 1 };
	return BezierCurveRecursive(6, &data[0], f);
}

void CatmullRomSplines(int size, Vec2* points)
{
	for (int i = 0; i < size - 2; i++)
	{
		Vec2 vel = 0.5 * (points[i + 2] - points[i + 1] + points[i + 1] - points[i + 0]);
	}
}


CubicBezierCurve2D::CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos)
	: m_pos0(startPos)
	, m_pos1(guidePos1)
	, m_pos2(guidePos2)
	, m_pos3(endPos)
{
}

CubicBezierCurve2D::CubicBezierCurve2D(const CubicHermiteCurve2D& fromHermite)
	: m_pos0(fromHermite.m_startPos)
	, m_pos1(fromHermite.m_startPos + fromHermite.m_startVel * (1.f / 3.f))
	, m_pos2(fromHermite.m_endPos - fromHermite.m_endVel * (1.f / 3.f))
	, m_pos3(fromHermite.m_endPos)
{
}

Vec2 CubicBezierCurve2D::EvalAtParametric(float fraction) const
{
	Vec2 tempData[4] = { m_pos0, m_pos1, m_pos2, m_pos3 };
	return BezierCurveT<4, Vec2>(&tempData[0], fraction);
}

float CubicBezierCurve2D::GetApproximateLength(int numSubdivisions /*= 64*/) const
{
	float fragment = 1.0f / float(numSubdivisions);
	Vec2 point0 = m_pos0;
	Vec2 point1;
	float length = 0.0f;
	for (int i = 1; i <= numSubdivisions; i++)
	{
		point1 = EvalAtParametric(fragment * float(i));
		length += (point1 - point0).GetLength();
		point0 = point1;
	}
	return length;
}

Vec2 CubicBezierCurve2D::EvalAtApproximateDistance(float distanceonCurve, int numSubdivisions /*= 64*/) const
{
	float evalLength = distanceonCurve;

	float fragment = 1.0f / float(numSubdivisions);
	Vec2 point0 = m_pos0;
	Vec2 point1;
	float length = 0.0f;
	for (int i = 1; i <= numSubdivisions; i++)
	{
		float oldLength = length;

		point1 = EvalAtParametric(fragment * float(i));
		length += (point1 - point0).GetLength();

		if (length >= evalLength)
			return Lerp(point0, point1, GetFractionWithin(evalLength, oldLength, length));

		point0 = point1;
	}
	return m_pos3;
}

CubicHermiteCurve2D::CubicHermiteCurve2D(const CubicBezierCurve2D& fromBezier)
	: m_startPos(fromBezier.m_pos0)
	, m_startVel(3 * (fromBezier.m_pos1 - fromBezier.m_pos0))
	, m_endPos(fromBezier.m_pos3)
	, m_endVel(3 * (fromBezier.m_pos3 - fromBezier.m_pos2))
{
}

CubicHermiteCurve2D::CubicHermiteCurve2D(Vec2 startPos, Vec2 startVel, Vec2 endPos, Vec2 endVel)
	: m_startPos(startPos)
	, m_startVel(startVel)
	, m_endPos(endPos)
	, m_endVel(endVel)
{
}

Vec2 CubicHermiteCurve2D::EvalAtParametric(float fraction) const
{
	return CubicBezierCurve2D(*this).EvalAtParametric(fraction);
}

float CubicHermiteCurve2D::GetApproximateLength(int numSubdivisions /*= 64*/) const
{
	return CubicBezierCurve2D(*this).GetApproximateLength(numSubdivisions);
}

Vec2 CubicHermiteCurve2D::EvalAtApproximateDistance(float distanceonCurve, int numSubdivisions /*= 64*/) const
{
	return CubicBezierCurve2D(*this).EvalAtApproximateDistance(distanceonCurve, numSubdivisions);
}

CatmullRomSpline::CatmullRomSpline()
{
	m_position.reserve(4);
	m_velocity.reserve(4);
	m_position.resize(1);
	m_velocity.resize(1);
}

CatmullRomSpline::CatmullRomSpline(size_t size, Vec2* data)
{
	m_size = size - 1;

	m_position.resize(size);
	m_velocity.resize(size);

	memcpy(m_position.data(), data, sizeof(Vec2) * size);

	m_velocity[0] = Vec2::ZERO;
	m_velocity[size - 1] = Vec2::ZERO;

	for (size_t i = 1; i < size - 1; i++)
	{
		m_velocity[i] = (m_position[i + 1] - m_position[i - 1]) * 0.5f;
	}
}

void CatmullRomSpline::AddPoint(Vec2 point)
{
	if (m_size > 0)
		m_velocity[m_size] = (point - m_position[m_size - 1]) * 0.5f;
	m_size += 1;
	m_position.push_back(point);
	m_velocity.push_back(Vec2::ZERO);
}

Vec2 CatmullRomSpline::EvalAtParametric(float fraction) const
{
	if (m_size == 0)
		return m_position[0];

	if (fraction == 1.0f)
		return m_position[m_position.size() - 1];

	fraction *= (float) m_size;
	int splineIdx = Floor(fraction);
	float subFraction = fraction - Floor(fraction);

	CubicHermiteCurve2D curve = CubicHermiteCurve2D(m_position[splineIdx], m_velocity[splineIdx], m_position[splineIdx + 1], m_velocity[splineIdx + 1]);
	return curve.EvalAtParametric(subFraction);
}

float CatmullRomSpline::GetApproximateLength(int numSubdivisions /*= 64*/) const
{
	if (m_size == 0)
		return 0.0f;

	float length = 0;

	for (size_t splineIdx = 0; splineIdx < m_size; splineIdx++)
	{
		CubicHermiteCurve2D curve = CubicHermiteCurve2D(m_position[splineIdx], m_velocity[splineIdx], m_position[splineIdx + 1], m_velocity[splineIdx + 1]);
		length += curve.GetApproximateLength(numSubdivisions);
	}

	return length;
}

Vec2 CatmullRomSpline::EvalAtApproximateDistance(float distanceonCurve, int numSubdivisions /*= 64*/) const
{
	if (m_size == 0)
		return m_position[0];

	for (size_t splineIdx = 0; splineIdx < m_size; splineIdx++)
	{
		CubicHermiteCurve2D curve = CubicHermiteCurve2D(m_position[splineIdx], m_velocity[splineIdx], m_position[splineIdx + 1], m_velocity[splineIdx + 1]);

		float curveLen = curve.GetApproximateLength(numSubdivisions);

		if (distanceonCurve > curveLen)
		{
			distanceonCurve -= curveLen;
			continue;
		}
		else
		{
			return curve.EvalAtApproximateDistance(distanceonCurve, numSubdivisions);
		}
	}

	return m_position[m_position.size() - 1];
}

