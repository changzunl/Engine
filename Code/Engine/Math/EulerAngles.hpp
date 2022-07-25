#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Matrix.hpp"

struct EulerAngles
{
public:
	static EulerAngles FromMatrix(const Mat4x4& mat);
	static EulerAngles FromMatrix(const Mat3x3& mat);

	EulerAngles() = default;
	EulerAngles(float yawDeg, float pitchDeg, float rollDeg);

	void   GetVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const;
	Vec3   GetVectorXForward() const;
	Vec3   GetVectorYLeft() const;
	Vec3   GetVectorZUp() const;
	Mat4x4 GetMatrix_XFwd_YLeft_ZUp() const;

	void   SetFromText(const char* text);

	bool   operator==(const EulerAngles& compare) const;		// EulerAngles == EulerAngles
	bool   operator!=(const EulerAngles& compare) const;		// EulerAngles != EulerAngles

public:
	float m_yawDegrees   = 0.0f;
	float m_pitchDegrees = 0.0f;
	float m_rollDegrees  = 0.0f;
};
