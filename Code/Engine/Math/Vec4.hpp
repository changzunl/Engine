#pragma once

struct Vec2;
struct Vec3;

//-----------------------------------------------------------------------------------------------
struct Vec4
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

public:
	// Construction/Destruction
	~Vec4();                                                                           // destructor (do nothing)
	Vec4();                                                                            // default constructor (do nothing)
	Vec4(const Vec4& copyFrom);                                                        // copy constructor (from another Vec3)
	Vec4(const Vec3& vec3, float initialW);                                            // copy constructor (from another Vec3)
	explicit Vec4(float initialX, float initialY, float initialZ, float initialW);     // explicit constructor (from x, y, z)

	const float* data() const;
	float* data();

	float Dot(const Vec4& b) const;

	// Operators (const)
	bool        operator==(const Vec4& compare) const;      // Vec3 == Vec3
	bool        operator!=(const Vec4& compare) const;      // Vec3 != Vec3
	const Vec4  operator+(const Vec4& vecToAdd) const;      // Vec3 + Vec3
	const Vec4  operator-(const Vec4& vecToSubtract) const; // Vec3 - Vec3
	const Vec4  operator-() const;                          // -Vec3, i.e. "unary negation"
	const Vec4  operator*(float uniformScale) const;        // Vec3 * float
	const Vec4  operator*(const Vec4& vecToMultiply) const; // Vec3 * Vec3
	const Vec4  operator/(float inverseScale) const;        // Vec3 / float

	// Operators (self-mutating / non-const)
	void        operator+=(const Vec4& vecToAdd);           // Vec3 += Vec3
	void        operator-=(const Vec4& vecToSubtract);      // Vec3 -= Vec3
	void        operator*=(const float uniformScale);       // Vec3 *= float
	void        operator/=(const float uniformDivisor);     // Vec3 /= float
	void        operator=(const Vec4& copyFrom);            // Vec3 = Vec3

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend const Vec4 operator*(float uniformScale, const Vec4& vecToScale); // float * Vec3
};


