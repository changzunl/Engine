#include "Engine/Animation/Quaternion.hpp"

#include "Engine/Math/MathUtils.hpp"
#include <math.h>


Quaternion Quaternion::FromAxisAndAngle(const Vec3& axis, float angle)
{
	float w = cosf(angle * 0.5f);
	Vec3 vec = axis * sinf(angle * 0.5f);
	return Quaternion(vec.x, vec.y, vec.z, w);
}

Quaternion Quaternion::FromEuler(const EulerAngles& euler)
{
	return FromMatrix(Mat3x3::GetSubMatrix(euler.GetMatrix_XFwd_YLeft_ZUp(), 3, 3));
}

Quaternion Quaternion::FromMatrix(const Mat3x3& mat)
{
	// from: assimp quaternion.h
	Quaternion q;
	float t = mat[0][0] + mat[1][1] + mat[2][2];

	// large enough
	if (t > 0.0f)
	{
		float s = sqrtf(1.0f + t) * 2.0f;
		q.x = (mat[1][2] - mat[2][1]) / s;
		q.y = (mat[2][0] - mat[0][2]) / s;
		q.z = (mat[0][1] - mat[1][0]) / s;
		q.w = 0.25f * s;
	} // else we have to check several cases
	else if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2])
	{
		// Column 0:
		float s = sqrtf(1.0f + mat[0][0] - mat[1][1] - mat[2][2]) * 2.0f;
		q.x = 0.25f * s;
		q.y = (mat[0][1] + mat[1][0]) / s;
		q.z = (mat[2][0] + mat[0][2]) / s;
		q.w = (mat[1][2] - mat[2][1]) / s;
	}
	else if (mat[1][1] > mat[2][2])
	{
		// Column 1:
		float s = sqrtf(1.0f + mat[1][1] - mat[0][0] - mat[2][2]) * 2.0f;
		q.x = (mat[0][1] + mat[1][0]) / s;
		q.y = 0.25f * s;
		q.z = (mat[1][2] + mat[2][1]) / s;
		q.w = (mat[2][0] - mat[0][2]) / s;
	}
	else
	{
		// Column 2:
		float s = sqrtf(1.0f + mat[2][2] - mat[0][0] - mat[1][1]) * 2.0f;
		q.x = (mat[2][0] + mat[0][2]) / s;
		q.y = (mat[1][2] + mat[2][1]) / s;
		q.z = 0.25f * s;
		q.w = (mat[0][1] - mat[1][0]) / s;
	}

	return q;
}

Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float alpha)
{
	Quaternion qOut;

	// calc cosine theta
	float cosom = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

	// adjust signs (if necessary)
	Quaternion end = q2;
	if (cosom < 0.0f)
	{
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	float sclp, sclq;
	if ((1.0f - cosom) > 0.0001f) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float omega, sinom;
		omega = acosf(cosom); // extract theta from dot product's cos theta
		sinom = sinf(omega);
		sclp = sinf((1.0f - alpha) * omega) / sinom;
		sclq = sinf(alpha * omega) / sinom;
	}
	else
	{
		// Very close, do linear interp (because it's faster)
		sclp = 1.0f - alpha;
		sclq = alpha;
	}

	qOut.x = sclp * q1.x + sclq * end.x;
	qOut.y = sclp * q1.y + sclq * end.y;
	qOut.z = sclp * q1.z + sclq * end.z;
	qOut.w = sclp * q1.w + sclq * end.w;

	return qOut;
}

Quaternion Quaternion::GetRotationFromTo(const Vec3& fromNormalized, const Vec3& toNormalized)
{
	float angleRad = acosf(fromNormalized.Dot(toNormalized));
	Vec3 axis = fromNormalized.Cross(toNormalized);
	return Quaternion::FromAxisAndAngle(axis, angleRad);
}

Mat4x4 Quaternion::GetMatrix() const
{
	Mat4x4 mat;
	mat[0][0] = 1.0f - 2.0f * (y * y + z * z);
	mat[1][0] = 2.0f * (x * y - z * w);
	mat[2][0] = 2.0f * (x * z + y * w);
	mat[0][1] = 2.0f * (x * y + z * w);
	mat[1][1] = 1.0f - 2.0f * (x * x + z * z);
	mat[2][1] = 2.0f * (y * z - x * w);
	mat[0][2] = 2.0f * (x * z - y * w);
	mat[1][2] = 2.0f * (y * z + x * w);
	mat[2][2] = 1.0f - 2.0f * (x * x + y * y);

	return mat;
}

void Quaternion::GetAngleAxis(Vec3& axis, float& angle) const
{
	angle = acosf(w) * 2.0f;
	float sinInv = 1.0f / sinf(angle * 0.5f);
	axis.x = x * sinInv;
	axis.y = y * sinInv;
	axis.z = z * sinInv;
}

EulerAngles Quaternion::GetEulerAngles() const
{
	return EulerAngles::FromMatrix(GetMatrix());
}

void Quaternion::SetIdentity()
{
	*this = Quaternion();
}

void Quaternion::Normalize()
{
	float mag = sqrtf(x * x + y * y + z * z + w * w);
	if (mag)
	{
		float invMag = 1.0f / mag;
		x *= invMag;
		y *= invMag;
		z *= invMag;
		w *= invMag;
	}
}


TransformQuat TransformQuat::DecomposeAffineMatrix(Mat4x4 mat)
{
	TransformQuat transform;

	// extract translation
	transform.m_position = mat.GetTranslation3D();

	// extract scale
	transform.m_scaling.x = mat.GetIBasis3D().GetLength();
	transform.m_scaling.y = mat.GetJBasis3D().GetLength();
	transform.m_scaling.z = mat.GetKBasis3D().GetLength();

	if (mat.GetDeterminant() < 0) transform.m_scaling = -transform.m_scaling;

	if (transform.m_scaling.x) mat.SetIJK3D(mat.GetIBasis3D() / transform.m_scaling.x, mat.GetJBasis3D(), mat.GetKBasis3D());
	if (transform.m_scaling.y) mat.SetIJK3D(mat.GetIBasis3D(), mat.GetJBasis3D() / transform.m_scaling.y, mat.GetKBasis3D());
	if (transform.m_scaling.z) mat.SetIJK3D(mat.GetIBasis3D(), mat.GetJBasis3D(), mat.GetKBasis3D() / transform.m_scaling.z);

	transform.m_rotation = Quaternion::FromMatrix(Mat3x3::GetSubMatrix(mat, 3, 3));

	return transform;
}

TransformQuat::TransformQuat()
	: m_position()
	, m_rotation()
	, m_scaling(1.0f, 1.0f, 1.0f)
{
}

TransformQuat::~TransformQuat()
{

}

Mat4x4 TransformQuat::GetMatrix() const
{
	Mat4x4 mat;
	mat.AppendTranslation3D(m_position);
	mat.Append(m_rotation.GetMatrix());
	mat.AppendScaleNonUniform3D(m_scaling);
	return mat;
}

Vec3 TransformQuat::GetForward() const
{
	return m_rotation.GetEulerAngles().GetVectorXForward();
}

