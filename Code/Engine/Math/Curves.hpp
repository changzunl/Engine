#pragma once

#include "MathUtils.hpp"
#include <vector>
#include <iostream>

// Easing function
float SmoothStart2(float fraction);
float SmoothStart3(float fraction);
float SmoothStart4(float fraction);
float SmoothStart5(float fraction);
float SmoothStart6(float fraction);
float SmoothStop2(float fraction);
float SmoothStop3(float fraction);
float SmoothStop4(float fraction);
float SmoothStop5(float fraction);
float SmoothStop6(float fraction);
float SmoothStep3(float fraction);
float SmoothStep5(float fraction);
float Hesitate3(float fraction);
float Hesitate5(float fraction);

class CubicBezierCurve2D;
class CubicHermiteCurve2D;

class CubicBezierCurve2D
{
	friend class CubicHermiteCurve2D;

public:
	CubicBezierCurve2D(Vec2 startPos, Vec2 guidePos1, Vec2 guidePos2, Vec2 endPos);
	CubicBezierCurve2D(const CubicHermiteCurve2D& fromHermite);

	Vec2  EvalAtParametric(float fraction) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2  EvalAtApproximateDistance(float distanceonCurve, int numSubdivisions = 64) const;

public:
	Vec2 m_pos0;
	Vec2 m_pos1;
	Vec2 m_pos2;
	Vec2 m_pos3;
};

class CubicHermiteCurve2D
{
	friend class CubicBezierCurve2D;

public:
	CubicHermiteCurve2D(Vec2 startPos, Vec2 startVel, Vec2 endPos, Vec2 endVel);
	CubicHermiteCurve2D(const CubicBezierCurve2D& fromHermite);

	Vec2  EvalAtParametric(float fraction) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2  EvalAtApproximateDistance(float distanceonCurve, int numSubdivisions = 64) const;

public:
	Vec2 m_startPos;
	Vec2 m_startVel;
	Vec2 m_endPos;
	Vec2 m_endVel;
};

class CatmullRomSpline
{
public:
	CatmullRomSpline();
	CatmullRomSpline(size_t size, Vec2* data);

	void AddPoint(Vec2 point);

	Vec2  EvalAtParametric(float fraction) const;
	float GetApproximateLength(int numSubdivisions = 64) const;
	Vec2  EvalAtApproximateDistance(float distanceonCurve, int numSubdivisions = 64) const;

public:
	size_t m_size = 0;
	std::vector<Vec2> m_position;
	std::vector<Vec2> m_velocity;
};


template<typename T>
T BezierCurveRecursive(int step, T* pointsTempData, float fraction)
{
	if (step == 1)
		return pointsTempData[0];

	for (int i = 0; i < step - 1; i++)
		pointsTempData[i] = Lerp(pointsTempData[i], pointsTempData[i + 1], fraction);
	return BezierCurveRecursive(step - 1, pointsTempData, fraction);
}

template<typename T>
T BezierCurve(int step, const T* points, float fraction)
{
	T* pointsTempData = new T[step];
	memcpy(pointsTempData, points, step * sizeof(T));

	T result = BezierCurveRecursive(step, pointsTempData, fraction);
	delete[] pointsTempData;
	return result;
}

template<typename T>
T BezierCurve(const T& point1, const T& point2, const T& point3, float fraction)
{
	T points[3] = { point1, point2, point3 };
	return BezierCurve(3, &points[0], fraction);
}
