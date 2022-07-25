#include "EulerAngles.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

EulerAngles EulerAngles::FromMatrix(const Mat4x4& mat)
{
	return FromMatrix(Mat3x3::GetSubMatrix(mat, 3, 3));
}

EulerAngles EulerAngles::FromMatrix(const Mat3x3& mat)
{
	EulerAngles euler;

	{
		// extract rotation
		// What is it? from http://nghiaho.com/?page_id=846
		float m00 = mat[0][0];
		float m01 = mat[0][1];
		float m02 = mat[0][2];
		float m12 = mat[1][2];
		float m22 = mat[2][2];

		euler.m_yawDegrees = ConvertRadiansToDegrees(atan2f(m01, m00));
		euler.m_pitchDegrees = ConvertRadiansToDegrees(atan2f(-m02, sqrtf(m12 * m12 + m22 * m22)));
		euler.m_rollDegrees = ConvertRadiansToDegrees(atan2f(m12, m22));
	}

	return euler;
}

EulerAngles::EulerAngles(float yawDeg, float pitchDeg, float rollDeg)
	: m_yawDegrees(yawDeg)
	, m_pitchDegrees(pitchDeg)
	, m_rollDegrees(rollDeg)
{
}

void EulerAngles::GetVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);

	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	Vec3 iBasis(cy * cp, sy * cp, -sp);
	Vec3 jBasis(-sy * cr + cy * sp * sr, cy * cr + sy * sp * sr, cp * sr);
	Vec3 kBasis(sy * sr + cy * sp * cr, -cy * sr + sy * sp * cr, cp * cr);

	out_forwardIBasis = iBasis;
	out_leftJBasis    = jBasis;
	out_upKBasis      = kBasis;
}

Vec3 EulerAngles::GetVectorXForward() const
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);

	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	return Vec3(cy * cp, sy * cp, -sp);
}

Vec3 EulerAngles::GetVectorYLeft() const
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);

	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	return Vec3(-sy * cr + cy * sp * sr, cy * cr + sy * sp * sr, cp * sr);
}

Vec3 EulerAngles::GetVectorZUp() const
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);

	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	return Vec3(sy * sr + cy * sp * cr, -cy * sr + sy * sp * cr, cp * cr);
}

Mat4x4 EulerAngles::GetMatrix_XFwd_YLeft_ZUp() const
{
#ifdef USE_MATRIX_COMPOSE
	Mat4x4 mYaw =   Mat4x4::CreateZRotationDegrees(m_yawDegrees);
	Mat4x4 mPitch = Mat4x4::CreateYRotationDegrees(m_pitchDegrees);
	Mat4x4 mRoll  = Mat4x4::CreateXRotationDegrees(m_rollDegrees);

	Mat4x4 mResult;
	mResult.Append(mYaw);
	mResult.Append(mPitch);
	mResult.Append(mRoll);

	return mResult;
#else
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);

	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	float cr = CosDegrees(m_rollDegrees);
	float sr = SinDegrees(m_rollDegrees);

	Vec3 iBasis(cy * cp, sy * cp, -sp);
	Vec3 jBasis(-sy * cr + cy * sp * sr, cy * cr + sy * sp * sr, cp * sr);
	Vec3 kBasis(sy * sr + cy * sp * cr, -cy * sr + sy * sp * cr, cp * cr);

	Mat4x4 mat;

	mat.SetIJK3D(iBasis, jBasis, kBasis);

	return mat;
#endif
}

void EulerAngles::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() == 3, Stringf("Illegal text data for vec3: %s", text));

	m_yawDegrees   = (float)atof(TrimString(strList[0]).c_str());
	m_pitchDegrees = (float)atof(TrimString(strList[1]).c_str());
	m_rollDegrees  = (float)atof(TrimString(strList[2]).c_str());
}

bool EulerAngles::operator!=(const EulerAngles& compare) const
{
	return !operator==(compare);
}

bool EulerAngles::operator==(const EulerAngles& compare) const
{
	return m_yawDegrees   == compare.m_yawDegrees
		&& m_pitchDegrees == compare.m_pitchDegrees
		&& m_rollDegrees  == compare.m_rollDegrees;
}

