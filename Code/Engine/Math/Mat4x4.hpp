#pragma once

struct Vec2;
struct Vec3;
struct Vec4;

typedef unsigned long long uint64_t;

//------------------------------------------------------------------------------------------------
// A 4x4 Homogeneous 2D/3D transformation matrix, stored basis-major in memory (Ix,Iy,Iz,Iw,Jx,Jy...).
// 
// Note: we specifically DO NOT provide an operator* overload,since doing so would require a
// decision to commit the Mat44 to only work correctly with EITHER column-major or row-major style
// style notation.  They demand two different ways of writing operator*, and in order to implement
// an operator*, we are forced to make a notational committment.  This is certainly ambiguous to the
// reader and, at the very least, potentially confusing.  Instead, we prefer to use method names,
// such as "Append", which are more neutral (e.g. multiply a new matrix "on the right in column-
// notation / on the left in row-notation".
// 
struct Mat4x4 
{
	static const Mat4x4 IDENTITY;

	enum {  Ix,Iy,Iz,Iw,  Jx,Jy,Jz,Jw,  Kx,Ky,Kz,Kw,  Tx,Ty,Tz,Tw,  MATRIX_SIZE  }; // index nicknames, [0] through [15]
	float m_values[16]; // stored in "basis major" order (Ix,Iy,Iz,Iw,Jx...) - translation in [12,13,14]

	Mat4x4(); // Default constructor is IDENTITY matrix!
	explicit Mat4x4(const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translation2D);
	explicit Mat4x4(const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translation3D);
	explicit Mat4x4(const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D);
	explicit Mat4x4(const float* sixteenValuesBasisMajor);

	const float* data() const;
	float* data();

	static const Mat4x4  CreateTranslation2D(const Vec2& translationXY);
	static const Mat4x4  CreateTranslation3D(const Vec3& translationXYZ);
	static const Mat4x4  CreateUniformScale2D(float uniformScaleXY);
	static const Mat4x4  CreateUniformScale3D(float uniformScaleXYZ);
	static const Mat4x4  CreateNonUniformScale2D(const Vec2& nonUniformScaleXY);
	static const Mat4x4  CreateNonUniformScale3D(const Vec3& nonUniformScaleXYZ);
	static const Mat4x4  CreateZRotationDegrees(float rotationDegreesAboutZ);
	static const Mat4x4  CreateYRotationDegrees(float rotationDegreesAboutY);
	static const Mat4x4  CreateXRotationDegrees(float rotationDegreesAboutX);
	static const Mat4x4  CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	static const Mat4x4  CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar);
	
	const Vec2      TransformVectorQuantity2D(const Vec2& vectorQuantityXY) const;     // assumes z=0, w=0 
	const Vec3      TransformVectorQuantity3D(const Vec3& vectorQuantityXYZ) const;    // assumes w=0 
	const Vec2      TransformPosition2D(const Vec2& positionXY) const;                 // assumes z=0, w=1 
	const Vec3      TransformPosition3D(const Vec3& position3D) const;                 // assumes w=1 
	const Vec4      TransformHomogeneous3D(const Vec4& homogeneousPoint3D) const;      // w is provided
	
	float*          GetAsFloatArray();          // non-const (mutable) version 
	const float*    GetAsFloatArray() const;    // const version, used only when Mat44 is const
	const Vec2      GetIBasis2D() const;
	const Vec2      GetJBasis2D() const;
	const Vec2      GetTranslation2D()const;
	const Vec3      GetIBasis3D() const;
	const Vec3      GetJBasis3D() const;
	const Vec3      GetKBasis3D()const;
	const Vec3      GetTranslation3D() const;
	const Vec4      GetIBasis4D() const;
	const Vec4      GetJBasis4D()const;
	const Vec4      GetKBasis4D() const;
	const Vec4      GetTranslation4D() const;
	Mat4x4          GetOrthonormalInverse() const;   // Only works for orthonormal affine matrices
	float           GetDeterminant() const;


	void SetIdentity();
	void SetValues(const float* sixteenValuesBasisMajor);
	void SetTranslation2D(const Vec2& translationXY);     // Sets translationZ = 0.translationW = 1
	void SetTranslation3D(const Vec3& translationXYZ);    // Sets translationW = 1
	void SetIJ2D(const Vec2& iBasis2D, const Vec2& jBasis2D); // Sets z=o, w=0 for i & j; does not modify k or t
	void SetIJT2D(const Vec2& iBasis2D, const Vec2& jBasis2D, const Vec2& translationXY); // Sets z=0, w=0 for i,j,t; does not modify k
	void SetIJK3D(const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D);      // Sets w=0 for i,j,k
	void SetIJKT3D(const Vec3& iBasis3D, const Vec3& jBasis3D, const Vec3& kBasis3D, const Vec3& translationXYZ); // Sets w=0 for i,j,k,t 
	void SetIJKT4D(const Vec4& iBasis4D, const Vec4& jBasis4D, const Vec4& kBasis4D, const Vec4& translation4D);
	void Transpose();                     // Swaps columns with rows
	void Orthonormalize_XFwd_YLeft_ZUp(); // Forward is canonical, Up is secondary, Left tertiary

	void Append(const Mat4x4& appendThis);                             // multiply on right in column notation / on left in row notation 
	void AppendZRotation(float degreesRotationAboutZ);                // same as appending(*= in column notation) a z-rotation matrix
	void AppendYRotation(float degreesRotationAboutY);                // same as appending(*= in column notation) a y-rotation matrix
	void AppendXRotation(float degreesRotationAboutX);                // sane as appending(*= in column notation) a x-rotation matrix
	void AppendTranslation2D(const Vec2& translationXY);              // same as appending(*= in column notation) a translation matrix 
	void AppendTranslation3D(const Vec3& translationXYZ);             // same as appending(*= in column notation) a translation matrix
	void AppendScaleUniform2D(float uniformScaleXY);                 // K and T bases should remain unaffected
	void AppendScaleUniform3D(float uniformScaleXYZ);                 // translation should remain unaffected
	void AppendScaleNonUniform2D(const Vec2& nonUniformScaleXY);      // K and T bases should remain unaffected
	void AppendScaleNonUniform3D(const Vec3& nonuniformscaleXYZ );    // translation should remain unaffected


	// accessor return float[] at column
	const float* operator[](uint64_t column) const { return &m_values[column << 2]; }
	float* operator[](uint64_t column) { return &m_values[column << 2]; }

	const Mat4x4 operator*(const Mat4x4& matToMultiply) const;

private:
	// accessor returns reference at offset
	const float& m(int offset) const { return m_values[offset]; }
	float& m(int offset) { return m_values[offset]; }
};

