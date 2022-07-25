#pragma once
#include "Engine/Math/IntVec2.hpp"

struct Vec2;
struct Vec3;
struct IntVec2;

struct IntVec3
{
public:
	static const IntVec3 ZERO;
	static const IntVec3 ONE;
	int x = 0;
	int y = 0;
	int z = 0;

public:
	IntVec3() {}												// default constructor (do nothing)
	constexpr IntVec3(const IntVec3& copyFrom);							// copy constructor (from another intvec3)
	explicit IntVec3(const IntVec2& vec2, int initialZ = 0);	// from vec2 constructor (from another vec3)
	explicit IntVec3(const Vec2& vec2, float initialZ = 0.0f);	// from vec2 constructor (from another vec3)
	explicit IntVec3(const Vec3& vec3);				          	// from vec2 constructor (from another vec3)
	explicit constexpr IntVec3(int initialX, int initialY, int initialZ);	// explicit constructor (from x, y)

	const int* data() const;
	int* data();

	int              GetLengthSquared() const;
	float            GetLength() const;
	int              GetTaxicabLength() const;

	// Serialization
	void             SetFromText(const char* text);

	// Operators (const)
	bool		    operator==(const IntVec3& compare) const;		// IntVec3 == IntVec3
	bool		    operator!=(const IntVec3& compare) const;		// IntVec3 != IntVec3
	const IntVec3	operator+(const IntVec3& vecToAdd) const;		// IntVec3 + IntVec3
	const IntVec3	operator-(const IntVec3& vecToSubtract) const;	// IntVec3 - IntVec3
	const IntVec3	operator-() const;								// -IntVec3, i.e. "unary negation"
	const IntVec3	operator*(int uniformScale) const;			    // IntVec3 * float
	const IntVec3	operator*(const IntVec3& vecToMultiply) const;	// IntVec3 * IntVec3

	// Operators (self-mutating / non-const)
	void	    	operator+=(const IntVec3& vecToAdd);			// IntVec3 += IntVec3
	void	    	operator-=(const IntVec3& vecToSubtract);		// IntVec3 -= IntVec3
	void	    	operator*=(const int uniformScale);			// IntVec3 *= float
	void	    	operator=(const IntVec3& copyFrom);				// IntVec3 = IntVec3

	// Standalone "friend" functions that are conceptually, but not actually, part of IntVec3::
	friend const IntVec3 operator*(int uniformScale, const IntVec3& vecToScale);	// float * IntVec3
};

constexpr IntVec3::IntVec3(const IntVec3& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
	, z(copyFrom.z)
{
}

constexpr IntVec3::IntVec3(int initialX, int initialY, int initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

