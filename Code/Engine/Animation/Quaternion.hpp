#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Quaternion
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

public:
	static Quaternion FromAxisAndAngle(const Vec3& axis, float angleRad);
	static Quaternion FromEuler(const EulerAngles& euler);
	static Quaternion FromMatrix(const Mat3x3& mat);

	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float alpha);
	static Quaternion GetRotationFromTo(const Vec3& fromNormalized, const Vec3& toNormalized);

	Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quaternion(float _x, float _y, float _z, float _w = 1.0f) : x(_x), y(_y), z(_z), w(_w) {}

	Mat4x4      GetMatrix() const;
	void        GetAngleAxis(Vec3& axis, float& angle) const;
	EulerAngles GetEulerAngles() const;

	void        SetIdentity();
	void        Normalize();
};


class TransformQuat
{
public:
	/* This method decomposes an affine matrix with translation, rotation, POSITIVE scale and WITHOUT shear
	*/
	TransformQuat static DecomposeAffineMatrix(Mat4x4 mat);

	TransformQuat();
	~TransformQuat();

	Mat4x4 GetMatrix() const;
	Vec3   GetForward() const;

public:
	Vec3        m_position;
	Quaternion  m_rotation;
	Vec3        m_scaling;
};

