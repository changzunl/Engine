#pragma once

#include "Engine/Math/Mat4x4.hpp"

class Mat2x2;
class Mat3x3;


class Mat2x2
{
public:
	static const Mat2x2 IDENTITY;

private:
	enum { Ix, Iy, Jx, Jy, MATRIX_SIZE }; // index nicknames, [0] through [3]
	float m_values[2 * 2] = {};

public:
	static Mat2x2 GetDeterminantMatrix(const Mat3x3& mat, int detColumn);
	static Mat2x2 GetSubMatrix(const Mat3x3& mat, int ignoreColumn, int ignoreRow);

	Mat2x2();
	Mat2x2(float* data);

	void  SetIdentity();
	void  SetValues(const float* data);
	float GetDeterminant() const;

	// accessor return float[] at column
	const float* operator[](uint64_t column) const { return &m_values[column << 1]; }
	float* operator[](uint64_t column) { return &m_values[column << 1]; }

private:
	// accessor returns reference at offset
	const float& m(int offset) const { return m_values[offset]; }
	float& m(int offset) { return m_values[offset]; }
};


class Mat3x3
{
public:
	static const Mat3x3 IDENTITY;

private:
	enum { Ix, Iy, Iz, Jx, Jy, Jz, Kx, Ky, Kz, MATRIX_SIZE }; // index nicknames, [0] through [8]
	float m_values[3 * 3] = {};

public:
	static Mat3x3 GetDeterminantMatrix(const Mat4x4& mat, int detColumn);
	static Mat3x3 GetSubMatrix(const Mat4x4& mat, int ignoreColumn, int ignoreRow);

	Mat3x3();
	Mat3x3(float* data);

	void  SetIdentity();
	void  SetValues(const float* data);
	float GetDeterminant() const;

	// accessor return float[] at column
	const float* operator[](uint64_t column) const { return &m_values[column * 3]; }
	float* operator[](uint64_t column) { return &m_values[column * 3]; }

private:
	// accessor returns reference at offset
	const float& m(int offset) const { return m_values[offset]; }
	float& m(int offset) { return m_values[offset]; }
};

