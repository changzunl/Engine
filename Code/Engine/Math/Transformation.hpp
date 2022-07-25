#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat4x4.hpp"
#include "Engine/Math/EulerAngles.hpp"


class Transformation
{
public:
	/* This method decomposes an affine matrix with translation, rotation, POSITIVE scale and WITHOUT shear
	*/
	Transformation static DecomposeAffineMatrix(Mat4x4 mat);

	Transformation();
	~Transformation();

	Mat4x4 GetMatrix() const;
	Vec3   GetForward() const;

	const Vec2& GetPosition2D() const;
	Vec2& GetPosition2D();
	const float& GetRotation2D() const;
	float& GetRotation2D();
	const Vec2& GetScale2D() const;
	Vec2& GetScale2D();

public:
	Vec3        m_position;
	EulerAngles m_orientation;
	Vec3        m_scale;
};

