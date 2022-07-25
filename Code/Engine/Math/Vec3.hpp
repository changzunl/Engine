#pragma once

struct Vec2;

//-----------------------------------------------------------------------------------------------
struct Vec3
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	static const Vec3 ZERO;
	static const Vec3 ONE;
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

public:
	// Construction/Destruction
	~Vec3() {}												// destructor (do nothing)
	Vec3() {}												// default constructor (do nothing)
	Vec3(const Vec3& copyFrom);							// copy constructor (from another Vec3)
	explicit Vec3(float initialX, float initialY, float initialZ);		// explicit constructor (from x, y, z)
	explicit Vec3(const Vec2& fromVec, float initialZ = 0.0f);

	const float* data() const;
	float* data();

	float      GetLength() const;
	float      GetLengthXY() const;
	float      GetLengthSquared() const;
	float      GetLengthXYSquared() const;
	float      GetAngleAboutZRadians() const;
	float      GetAngleAboutZDegrees() const;
	const Vec3 GetRotatedAboutZRadians(float radians) const; // rotate around (0, 0)
	const Vec3 GetRotatedAboutZDegrees(float degrees) const; // rotate around (0, 0)
	const Vec3 GetClamped(float maxLength) const;
	const Vec3 GetNormalized() const;
	float      Dot(const Vec3& b) const;
	float      Dot2D(const Vec3& b) const;
	const Vec3 Cross(const Vec3& b) const;
	float      Cross2D(const Vec3& b) const;

	const float NormalizeAndGetPreviousLength();

	// Serialization
	void       SetFromText(const char* text);

	// Operators (const)
	bool		operator==(const Vec3& compare) const;		// Vec3 == Vec3
	bool		operator!=(const Vec3& compare) const;		// Vec3 != Vec3
	const Vec3	operator+(const Vec3& vecToAdd) const;		// Vec3 + Vec3
	const Vec3	operator+(float valToAdd) const;		// Vec3 + Vec3
	const Vec3	operator-(const Vec3& vecToSubtract) const;	// Vec3 - Vec3
	const Vec3	operator-(float valToSubtract) const;	// Vec3 - Vec3
	const Vec3	operator-() const;								// -Vec3, i.e. "unary negation"
	const Vec3	operator*(float uniformScale) const;			// Vec3 * float
	const Vec3	operator*(const Vec3& vecToMultiply) const;	// Vec3 * Vec3
	const Vec3	operator/(float inverseScale) const;			// Vec3 / float
	const Vec3	operator/(const Vec3& vecToDiv) const;			// Vec3 / float

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec3& vecToAdd);				// Vec3 += Vec3
	void		operator+=(float valToAdd);				// Vec3 += Vec3
	void		operator-=(const Vec3& vecToSubtract);		// Vec3 -= Vec3
	void		operator-=(float valToSubtract);		// Vec3 -= Vec3
	void		operator*=(const float uniformScale);			// Vec3 *= float
	void		operator*=(const Vec3& vecToMul);			// Vec3 *= float
	void		operator/=(const float uniformDivisor);		// Vec3 /= float
	void		operator/=(const Vec3& vecToDiv);		// Vec3 /= Vec3
	void		operator=(const Vec3& copyFrom);				// Vec3 = Vec3

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend const Vec3 operator*(float uniformScale, const Vec3& vecToScale);	// float * Vec3
};


