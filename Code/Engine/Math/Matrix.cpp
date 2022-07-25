#include "Engine/Math/Matrix.hpp"


const Mat2x2 Mat2x2::IDENTITY;

Mat2x2 Mat2x2::GetDeterminantMatrix(const Mat3x3& mat3, int detColumn)
{
	Mat2x2 mat;
	constexpr int remainValues[3][2] = { { 1, 2 }, { 0, 2 }, { 0, 1 } };
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			mat[i][j] = mat3[remainValues[detColumn][i]][remainValues[0][j]];

	return mat;
}

Mat2x2 Mat2x2::GetSubMatrix(const Mat3x3& mat3, int ignoreColumn, int ignoreRow)
{
	Mat2x2 mat;
	constexpr int remainValues[3][2] = { { 1, 2 }, { 0, 2 }, { 0, 1 } };
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			mat[i][j] = mat3[remainValues[ignoreColumn][i]][remainValues[ignoreRow][j]];

	return mat;
}

Mat2x2::Mat2x2()
{
	SetIdentity();
}

Mat2x2::Mat2x2(float* data)
{
	SetValues(data);
}

void Mat2x2::SetIdentity()
{
	constexpr float IDENTITY_VALUES[MATRIX_SIZE] = { 1.0f, 0.0f, 0.0f, 1.0f };
	SetValues(IDENTITY_VALUES);
}

void Mat2x2::SetValues(const float* data)
{
	for (int i = 0; i < MATRIX_SIZE; i++)
		m_values[i] = data[i];
}

float Mat2x2::GetDeterminant() const
{
	return m(Ix) * m(Jy) - m(Jx) * m(Iy);
}

const Mat3x3 Mat3x3::IDENTITY;

Mat3x3 Mat3x3::GetDeterminantMatrix(const Mat4x4& mat4, int detColumn)
{
	Mat3x3 mat;
	constexpr int remainValues[4][3] = { { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 }, { 0, 1, 2 } };
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] = mat4[remainValues[detColumn][i]][remainValues[0][j]];
	return mat;
}

Mat3x3 Mat3x3::GetSubMatrix(const Mat4x4& mat4, int ignoreColumn, int ignoreRow)
{
	Mat3x3 mat;
	constexpr int remainValues[4][3] = { { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 }, { 0, 1, 2 } };
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] = mat4[remainValues[ignoreColumn][i]][remainValues[ignoreRow][j]];
	return mat;
}

Mat3x3::Mat3x3()
{
	SetIdentity();
}

Mat3x3::Mat3x3(float* data)
{
	SetValues(data);
}

void Mat3x3::SetIdentity()
{
	constexpr float IDENTITY_VALUES[MATRIX_SIZE] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	SetValues(IDENTITY_VALUES);
}

void Mat3x3::SetValues(const float* data)
{
	for (int i = 0; i < MATRIX_SIZE; i++)
		m_values[i] = data[i];
}

float Mat3x3::GetDeterminant() const
{
	Mat2x2 mat20 = Mat2x2::GetDeterminantMatrix(*this, 0);
	Mat2x2 mat21 = Mat2x2::GetDeterminantMatrix(*this, 1);
	Mat2x2 mat22 = Mat2x2::GetDeterminantMatrix(*this, 2);

	return m(Ix) * mat20.GetDeterminant() - m(Jx) * mat21.GetDeterminant() + m(Kx) * mat22.GetDeterminant();
}


float Mat4x4::GetDeterminant() const
{
	Mat3x3 mat30 = Mat3x3::GetDeterminantMatrix(*this, 0);
	Mat3x3 mat31 = Mat3x3::GetDeterminantMatrix(*this, 1);
	Mat3x3 mat32 = Mat3x3::GetDeterminantMatrix(*this, 2);
	Mat3x3 mat33 = Mat3x3::GetDeterminantMatrix(*this, 3);

	return m(Ix) * mat30.GetDeterminant() - m(Jx) * mat31.GetDeterminant() + m(Kx) * mat32.GetDeterminant() - m(Tx) * mat33.GetDeterminant();
}

