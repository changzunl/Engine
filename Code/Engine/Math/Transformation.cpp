#include "Engine/Math/Transformation.hpp"

#include "Engine/Math/MathUtils.hpp"

#include <math.h>


Transformation Transformation::DecomposeAffineMatrix(Mat4x4 mat)
{
	Transformation transform;

	// extract translation
	transform.m_position = mat.GetTranslation3D();


	// extract scale
	transform.m_scale.x = mat.GetIBasis3D().GetLength();
	transform.m_scale.y = mat.GetJBasis3D().GetLength();
	transform.m_scale.z = mat.GetKBasis3D().GetLength();

	if (mat.GetDeterminant() < 0) transform.m_scale = -transform.m_scale;

	if (transform.m_scale.x) mat.SetIJK3D(mat.GetIBasis3D() / transform.m_scale.x, mat.GetJBasis3D(), mat.GetKBasis3D());
	if (transform.m_scale.y) mat.SetIJK3D(mat.GetIBasis3D(), mat.GetJBasis3D() / transform.m_scale.y, mat.GetKBasis3D());
	if (transform.m_scale.z) mat.SetIJK3D(mat.GetIBasis3D(), mat.GetJBasis3D(), mat.GetKBasis3D() / transform.m_scale.z);

	transform.m_orientation = EulerAngles::FromMatrix(mat);

	return transform;
}

Transformation::Transformation()
	: m_position()
	, m_orientation()
	, m_scale(1.0f, 1.0f, 1.0f)
{
}

Transformation::~Transformation()
{

}

Mat4x4 Transformation::GetMatrix() const
{
	Mat4x4 mat;
	mat.AppendTranslation3D(m_position);
	mat.Append(m_orientation.GetMatrix_XFwd_YLeft_ZUp());
	mat.AppendScaleNonUniform3D(m_scale);

	return mat;
}

Vec3 Transformation::GetForward() const
{
	return m_orientation.GetVectorXForward();
}

const Vec2& Transformation::GetPosition2D() const
{
	return *((const Vec2*)&m_position);
}

Vec2& Transformation::GetPosition2D()
{
	return *((Vec2*)&m_position);
}

const float& Transformation::GetRotation2D() const
{
	return *((const float*)&m_orientation.m_yawDegrees);
}

float& Transformation::GetRotation2D()
{
	return *((float*)&m_orientation.m_yawDegrees);
}

const Vec2& Transformation::GetScale2D() const
{
	return *((const Vec2*)&m_scale);
}

Vec2& Transformation::GetScale2D()
{
	return *((Vec2*)&m_scale);
}

