#include "Mat4x4.hpp"

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "MathUtils.hpp"

const Mat4x4 Mat4x4::IDENTITY;

Mat4x4::Mat4x4()
{
	SetIdentity();
}

Mat4x4::Mat4x4(const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D)
{
	SetIdentity();
	SetIJT2D(iBasis2D, jBasis2D, translation2D);
}

Mat4x4::Mat4x4(const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D)
{
	SetIdentity();
	SetIJKT3D(iBasis3D, jBasis3D, kBasis3D, translation3D);
}

Mat4x4::Mat4x4(const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D)
{
	SetIJKT4D(iBasis4D, jBasis4D, kBasis4D, translation4D);
}

Mat4x4::Mat4x4(const float* sixteenValuesBasisMajor)
{
	SetValues(sixteenValuesBasisMajor);
}

const float* Mat4x4::data() const
{
	return (const float*)this;
}

float* Mat4x4::data()
{
	return (float*)this;
}

const Mat4x4 Mat4x4::CreateTranslation2D(const Vec2& translationXY)
{
	Mat4x4 mat;
	mat.SetTranslation2D(translationXY);
	return mat;
}

const Mat4x4 Mat4x4::CreateTranslation3D(const Vec3& translationXYZ)
{
	Mat4x4 mat;
	mat.SetTranslation3D(translationXYZ);
	return mat;
}

const Mat4x4 Mat4x4::CreateUniformScale2D(float uniformScaleXY)
{
	return CreateNonUniformScale2D(Vec2(uniformScaleXY, uniformScaleXY));
}

const Mat4x4 Mat4x4::CreateUniformScale3D(float uniformScaleXYZ)
{
	return CreateNonUniformScale3D(Vec3(uniformScaleXYZ, uniformScaleXYZ, uniformScaleXYZ));
}

const Mat4x4 Mat4x4::CreateNonUniformScale2D(const Vec2& nonUniformScaleXY)
{
	static const Vec2 uniformX(1.0f, 0.0f);
	static const Vec2 uniformY(0.0f, 1.0f);
	return Mat4x4(uniformX * nonUniformScaleXY.x, uniformY * nonUniformScaleXY.y, Vec2());
}

const Mat4x4 Mat4x4::CreateNonUniformScale3D(const Vec3& nonUniformScaleXYZ)
{
	static const Vec3 uniformX(1.0f, 0.0f, 0.0f);
	static const Vec3 uniformY(0.0f, 1.0f, 0.0f);
	static const Vec3 uniformZ(0.0f, 0.0f, 1.0f);
	return Mat4x4(uniformX * nonUniformScaleXYZ.x, uniformY * nonUniformScaleXYZ.y, uniformZ * nonUniformScaleXYZ.z, Vec3());
}

const Mat4x4 Mat4x4::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	float c = CosDegrees(rotationDegreesAboutZ);
	float s = SinDegrees(rotationDegreesAboutZ);
	return Mat4x4(Vec3(c, s, 0.0f), Vec3(-s, c, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3());
}

const Mat4x4 Mat4x4::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	float c = CosDegrees(rotationDegreesAboutY);
	float s = SinDegrees(rotationDegreesAboutY);
	return Mat4x4(Vec3(c, 0.0f, -s), Vec3(0.0f, 1.0f, 0.0f), Vec3(s, 0.0f, c), Vec3());
}

const Mat4x4 Mat4x4::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	float c = CosDegrees(rotationDegreesAboutX);
	float s = SinDegrees(rotationDegreesAboutX);
	return Mat4x4(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, c, s), Vec3(0.0f, -s, c), Vec3());
}

const Vec2 Mat4x4::TransformVectorQuantity2D(const Vec2& vectorQuantityXY) const
{
	Vec4 vec4 = Vec4(vectorQuantityXY.x, vectorQuantityXY.y, 0.0f, 0.0f);
	Vec4 result = TransformHomogeneous3D(vec4);
	return Vec2(result.x, result.y);
}

const Vec3 Mat4x4::TransformVectorQuantity3D(const Vec3& vectorQuantityXYZ) const
{
	Vec4 vec4 = Vec4(vectorQuantityXYZ.x, vectorQuantityXYZ.y, vectorQuantityXYZ.z, 0.0f);
	Vec4 result = TransformHomogeneous3D(vec4);
	return Vec3(result.x, result.y, result.z);
}

const Vec2 Mat4x4::TransformPosition2D(const Vec2& positionXY) const
{
	Vec4 vec4 = Vec4(positionXY.x, positionXY.y, 0.0f, 1.0f);
	Vec4 result = TransformHomogeneous3D(vec4);
	return Vec2(result.x, result.y);
}

const Vec3 Mat4x4::TransformPosition3D(const Vec3& position3D) const
{
	Vec4 vec4 = Vec4(position3D.x, position3D.y, position3D.z, 1.0f);
	Vec4 result = TransformHomogeneous3D(vec4);
	return Vec3(result.x, result.y, result.z);
}

const Vec4 Mat4x4::TransformHomogeneous3D(const Vec4& homogeneousPoint3D) const
{
	const Vec4& vec4 = homogeneousPoint3D;
	Vec4 result;
	result.x = m_values[Ix] * vec4.x + m_values[Jx] * vec4.y + m_values[Kx] * vec4.z + m_values[Tx] * vec4.w;
	result.y = m_values[Iy] * vec4.x + m_values[Jy] * vec4.y + m_values[Ky] * vec4.z + m_values[Ty] * vec4.w;
	result.z = m_values[Iz] * vec4.x + m_values[Jz] * vec4.y + m_values[Kz] * vec4.z + m_values[Tz] * vec4.w;
	result.w = m_values[Iw] * vec4.x + m_values[Jw] * vec4.y + m_values[Kw] * vec4.z + m_values[Tw] * vec4.w;
	return result;
}

float* Mat4x4::GetAsFloatArray()
{
	return &m_values[0];
}

const float* Mat4x4::GetAsFloatArray() const
{
	return &m_values[0];
}

const Vec2 Mat4x4::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}

const Vec2 Mat4x4::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

const Vec2 Mat4x4::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

const Vec3 Mat4x4::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

const Vec3 Mat4x4::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

const Vec3 Mat4x4::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

const Vec3 Mat4x4::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

const Vec4 Mat4x4::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

const Vec4 Mat4x4::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

const Vec4 Mat4x4::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

const Vec4 Mat4x4::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

Mat4x4 Mat4x4::GetOrthonormalInverse() const
{
	Mat4x4 mRotation = *this;
	mRotation.SetTranslation3D(Vec3());
	mRotation.Transpose();

	Mat4x4 mTranslation;
	mTranslation.SetTranslation3D(-GetTranslation3D());

	Mat4x4 result;
	result.Append(mRotation);
	result.Append(mTranslation);
	return result;
}

void Mat4x4::SetIdentity()
{
	constexpr float IDENTITY_DATA[]{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
	};

	SetValues(&IDENTITY_DATA[0]);
}

void Mat4x4::SetValues(const float* sixteenValuesBasisMajor)
{
	for (int idx = 0; idx < MATRIX_SIZE; idx++)
	{
		m_values[idx] = sixteenValuesBasisMajor[idx];
	}
}

void Mat4x4::SetTranslation2D(const Vec2& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

void Mat4x4::SetTranslation3D(const Vec3& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.0f;
}

void Mat4x4::SetIJ2D(const Vec2& iBasis2D, const Vec2& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;
}

void Mat4x4::SetIJT2D(const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translationXY)
{
	SetIJ2D(iBasis2D, jBasis2D);
	SetTranslation2D(translationXY);
}

void Mat4x4::SetIJK3D(const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.0f;
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.0f;
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.0f;
}

void Mat4x4::SetIJKT3D(const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translationXYZ)
{
	SetIJK3D(iBasis3D, jBasis3D, kBasis3D);
	SetTranslation3D(translationXYZ);
}

void Mat4x4::SetIJKT4D(const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;
	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;
	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;
	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat4x4::Transpose()
{
	Mat4x4 result;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			result.m_values[y + x * 4] = m_values[x + y * 4];
		}
	}

	SetValues(result.m_values);
}

void Mat4x4::Orthonormalize_XFwd_YLeft_ZUp()
{
	Vec3 iBasis = GetIBasis3D();
	Vec3 jBasis = GetJBasis3D();
	Vec3 kBasis = GetKBasis3D();

	// Gram Schmidt
	iBasis = iBasis.GetNormalized();
	kBasis -= GetProjectedOnto3D(kBasis, iBasis);
	kBasis = kBasis.GetNormalized();
	jBasis -= GetProjectedOnto3D(jBasis, kBasis);
	jBasis -= GetProjectedOnto3D(jBasis, iBasis);
	jBasis = jBasis.GetNormalized();

	SetIJK3D(iBasis, jBasis, kBasis);
}

void Mat4x4::Append(const Mat4x4& mat4)
{
	Mat4x4 result;

	result.m_values[Ix] = m_values[Ix] * mat4.m_values[Ix] + m_values[Jx] * mat4.m_values[Iy] + m_values[Kx] * mat4.m_values[Iz] + m_values[Tx] * mat4.m_values[Iw];
	result.m_values[Iy] = m_values[Iy] * mat4.m_values[Ix] + m_values[Jy] * mat4.m_values[Iy] + m_values[Ky] * mat4.m_values[Iz] + m_values[Ty] * mat4.m_values[Iw];
	result.m_values[Iz] = m_values[Iz] * mat4.m_values[Ix] + m_values[Jz] * mat4.m_values[Iy] + m_values[Kz] * mat4.m_values[Iz] + m_values[Tz] * mat4.m_values[Iw];
	result.m_values[Iw] = m_values[Iw] * mat4.m_values[Ix] + m_values[Jw] * mat4.m_values[Iy] + m_values[Kw] * mat4.m_values[Iz] + m_values[Tw] * mat4.m_values[Iw];

	result.m_values[Jx] = m_values[Ix] * mat4.m_values[Jx] + m_values[Jx] * mat4.m_values[Jy] + m_values[Kx] * mat4.m_values[Jz] + m_values[Tx] * mat4.m_values[Jw];
	result.m_values[Jy] = m_values[Iy] * mat4.m_values[Jx] + m_values[Jy] * mat4.m_values[Jy] + m_values[Ky] * mat4.m_values[Jz] + m_values[Ty] * mat4.m_values[Jw];
	result.m_values[Jz] = m_values[Iz] * mat4.m_values[Jx] + m_values[Jz] * mat4.m_values[Jy] + m_values[Kz] * mat4.m_values[Jz] + m_values[Tz] * mat4.m_values[Jw];
	result.m_values[Jw] = m_values[Iw] * mat4.m_values[Jx] + m_values[Jw] * mat4.m_values[Jy] + m_values[Kw] * mat4.m_values[Jz] + m_values[Tw] * mat4.m_values[Jw];

	result.m_values[Kx] = m_values[Ix] * mat4.m_values[Kx] + m_values[Jx] * mat4.m_values[Ky] + m_values[Kx] * mat4.m_values[Kz] + m_values[Tx] * mat4.m_values[Kw];
	result.m_values[Ky] = m_values[Iy] * mat4.m_values[Kx] + m_values[Jy] * mat4.m_values[Ky] + m_values[Ky] * mat4.m_values[Kz] + m_values[Ty] * mat4.m_values[Kw];
	result.m_values[Kz] = m_values[Iz] * mat4.m_values[Kx] + m_values[Jz] * mat4.m_values[Ky] + m_values[Kz] * mat4.m_values[Kz] + m_values[Tz] * mat4.m_values[Kw];
	result.m_values[Kw] = m_values[Iw] * mat4.m_values[Kx] + m_values[Jw] * mat4.m_values[Ky] + m_values[Kw] * mat4.m_values[Kz] + m_values[Tw] * mat4.m_values[Kw];

	result.m_values[Tx] = m_values[Ix] * mat4.m_values[Tx] + m_values[Jx] * mat4.m_values[Ty] + m_values[Kx] * mat4.m_values[Tz] + m_values[Tx] * mat4.m_values[Tw];
	result.m_values[Ty] = m_values[Iy] * mat4.m_values[Tx] + m_values[Jy] * mat4.m_values[Ty] + m_values[Ky] * mat4.m_values[Tz] + m_values[Ty] * mat4.m_values[Tw];
	result.m_values[Tz] = m_values[Iz] * mat4.m_values[Tx] + m_values[Jz] * mat4.m_values[Ty] + m_values[Kz] * mat4.m_values[Tz] + m_values[Tz] * mat4.m_values[Tw];
	result.m_values[Tw] = m_values[Iw] * mat4.m_values[Tx] + m_values[Jw] * mat4.m_values[Ty] + m_values[Kw] * mat4.m_values[Tz] + m_values[Tw] * mat4.m_values[Tw];

	SetValues(&result.m_values[0]);
}

void Mat4x4::AppendZRotation(float degreesRotationAboutZ)
{
	Append(CreateZRotationDegrees(degreesRotationAboutZ));
}

void Mat4x4::AppendYRotation(float degreesRotationAboutY)
{
	Append(CreateYRotationDegrees(degreesRotationAboutY));
}

void Mat4x4::AppendXRotation(float degreesRotationAboutX)
{
	Append(CreateXRotationDegrees(degreesRotationAboutX));
}

void Mat4x4::AppendTranslation2D(const Vec2& translationXY)
{
	Append(CreateTranslation2D(translationXY));
}

void Mat4x4::AppendTranslation3D(const Vec3& translationXYZ)
{
	Append(CreateTranslation3D(translationXYZ));
}

void Mat4x4::AppendScaleUniform2D(float uniformScaleXY)
{
	Append(CreateUniformScale2D(uniformScaleXY));
}

void Mat4x4::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Append(CreateUniformScale3D(uniformScaleXYZ));
}

void Mat4x4::AppendScaleNonUniform2D(const Vec2& nonUniformScaleXY)
{
	Append(CreateNonUniformScale2D(nonUniformScaleXY));
}

void Mat4x4::AppendScaleNonUniform3D(const Vec3& nonUniformScaleXYZ)
{
	Append(CreateNonUniformScale3D(nonUniformScaleXYZ));
}

const Mat4x4 Mat4x4::operator*(const Mat4x4& matToMultiply) const
{
	Mat4x4 result = *this;
	result.Append(matToMultiply);
	return result;
}

const Mat4x4 Mat4x4::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat4x4 mat;
	Vec3 i;
	Vec3 j;
	Vec3 k;
	Vec3 t;

	float iRange = 1.0f / (right - left);
	float jRange = 1.0f / (top - bottom);
	float kRange = 1.0f / (zFar - zNear);

	i.x = 2.f * iRange;
	j.y = 2.f * jRange;
	k.z = kRange;
	t.x = (right + left) * -iRange;
	t.y = (top + bottom) * -jRange;
	t.z = zNear * -kRange;

	mat.SetIJKT3D(i, j, k, t);
	return mat;
}

const Mat4x4 Mat4x4::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat4x4 mat;
	Vec4 i;
	Vec4 j;
	Vec4 k;
	Vec4 t;

	float scale = CosDegrees(0.5f * fovYDegrees) / SinDegrees(0.5f * fovYDegrees);
	float zRange = 1.0f / (zFar - zNear);

	i.x = scale / aspect;
	j.y = scale;
	k.z = zFar * zRange;
	k.w = 1.0f;
	t.z = (zNear * zFar) * -zRange;

	mat.SetIJKT4D(i, j, k, t);
	return mat;
}

