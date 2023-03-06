#include "Vec2.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "MathUtils.hpp"
#include <math.h>

const Vec2 Vec2::ZERO = Vec2(0.0f, 0.0f);

const Vec2 Vec2::ONE  = Vec2(1.0f, 0.0f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

const float* Vec2::data() const
{
	return (const float*)this;
}

float* Vec2::data()
{
	return (float*)this;
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length)
{
	if (length == 0)
	{
		return Vec2();
	}

	return Vec2(cosf(orientationRadians), sinf(orientationRadians)) * length;
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length)
{
	return MakeFromPolarRadians(ConvertDegreesToRadians(orientationDegrees), length);
}

float Vec2::GetLength() const
{
	return sqrtf(GetLengthSquared());
}

float Vec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float Vec2::GetOrientationRadians() const
{
	float theta = atan2f(y, x);
	return theta;
}

float Vec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(GetOrientationRadians());
}

Vec2 const Vec2::GetRotated90Degrees() const
{
	Vec2 vec = Vec2(x, y);
	vec.Rotate90Degrees();
	return vec;
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const
{
	Vec2 vec = Vec2(x, y);
	vec.RotateMinus90Degrees();
	return vec;
}

Vec2 const Vec2::GetRotatedRadians(float radians) const
{
	Vec2 vec = Vec2(x, y);
	vec.RotateRadians(radians);
	return vec;
}

Vec2 const Vec2::GetRotatedDegrees(float degrees) const
{
	Vec2 vec = Vec2(x, y);
	vec.RotateDegrees(degrees);
	return vec;
}

Vec2 const Vec2::GetClamped(float maxLength) const
{
	Vec2 vec = Vec2(x, y);
	vec.ClampLength(maxLength);
	return vec;
}

Vec2 const Vec2::GetNormalized() const
{
	if (x == 0 && y == 0)
	{
		return Vec2(1.0f, 0.0f);
	}

	float scale = 1.f / sqrtf(GetLengthSquared());
	return Vec2(x * scale, y * scale);
}

Vec2 const Vec2::GetReflected(const Vec2& impactSurfaceNormal) const
{
	Vec2 vec = Vec2(x, y);
	vec.Reflect(impactSurfaceNormal);
	return vec;
}

float Vec2::Dot(const Vec2& another) const
{
	return x * another.x + y * another.y;
}

float Vec2::Cross(const Vec2& b) const
{
	return x * b.y - y * b.x;
}

void Vec2::SetOrientationRadians(float orientationRadians)
{
	float length = GetLength();
	if (length == 0)
	{
		x = 0;
		y = 0;
		return;
	}

	float theta = orientationRadians;
	x = length * cosf(theta);
	y = length * sinf(theta);
}

void Vec2::SetOrientationDegrees(float orientationDegrees)
{
	SetOrientationRadians(ConvertDegreesToRadians(orientationDegrees));
}

void Vec2::SetPolarRadians(float orientationRadians, float length)
{
	if (length == 0)
	{
		x = 0;
		y = 0;
		return;
	}

	float theta = orientationRadians;
	x = length * cosf(theta);
	y = length * sinf(theta);
}

void Vec2::SetPolarDegrees(float orientationDegrees, float length)
{
	SetPolarRadians(ConvertDegreesToRadians(orientationDegrees), length);
}

void Vec2::Rotate90Degrees()
{
	float oldx = x;
	x = -y;
	y = oldx;
}

void Vec2::RotateMinus90Degrees()
{
	float oldy = y;
	y = -x;
	x = oldy;
}

void Vec2::RotateRadians(float radians)
{
	float c = cosf(radians);
	float s = sinf(radians);
	float newX = x * c + y * -s;
	float newY = x * s + y *  c;

	x = newX;
	y = newY;
}

void Vec2::RotateDegrees(float degrees)
{
	RotateRadians(ConvertDegreesToRadians(degrees));
}

void Vec2::SetLength(float length)
{
	if (x == 0 && y == 0)
	{
		x = length;
		return;
	}

	float scale = length / GetLength();
	x *= scale;
	y *= scale;
}

void Vec2::ClampLength(float maxLength)
{
	if (maxLength <= 0)
	{
		x = 0;
		y = 0;
		return;
	}

	float lengthSq = GetLengthSquared();
	if (lengthSq <= maxLength * maxLength)
	{
		return;
	}

	float scale = maxLength / sqrtf(lengthSq);
	x *= scale;
	y *= scale;
}

float FastInvSqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long*)&y;                       // evil floating point bit level hacking
	i = 0x5f3759df - (i >> 1);               // what the fuck? 
	y = *(float*)&i;
	y = y * (threehalfs - (x2 * y * y));   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

void Vec2::Normalize()
{
	if (x == 0 && y == 0)
	{
		x = 1;
		return;
	}

	float scale = 1.f / sqrtf(x * x + y * y);
	x *= scale;
	y *= scale;
}

float Vec2::NormalizeAndGetPreviousLength()
{
	if (x == 0 && y == 0)
	{
		x = 1;
		return 0;
	}

	float length = sqrtf(x * x + y * y);
	float scale = 1.f / length;
	x *= scale;
	y *= scale;
	return length;
}

void Vec2::Reflect(const Vec2& impactSurfaceNormal)
{
	*this -= 2.0f * DotProduct2D(*this, impactSurfaceNormal) * impactSurfaceNormal;
}

void Vec2::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() == 2, Stringf("Illegal text data for vec2: %s", text));

	x = (float) atof(TrimString(strList[0]).c_str());
	y = (float) atof(TrimString(strList[1]).c_str());
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2(x + vecToAdd.x, y + vecToAdd.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2(x * uniformScale, y * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2(x * vecToMultiply.x, y * vecToMultiply.y);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2(x / inverseScale, y / inverseScale);
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/(const Vec2& inverseScale) const
{
	return Vec2(x / inverseScale.x, y / inverseScale.y);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator+(float other) const
{
	return Vec2(x + other, y + other);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-(float other) const
{
    return Vec2(x - other, y - other);
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=(float other)
{
	x += other;
	y += other;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=(float other)
{
    x -= other;
    y -= other;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=(const Vec2& vecDivisor)
{
	x /= vecDivisor.x;
	y /= vecDivisor.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

//-----------------------------------------------------------------------------------------------
const Vec2 operator+(float uniformScale, const Vec2& vecToScale)
{
    return Vec2(uniformScale + vecToScale.x, uniformScale + vecToScale.y);
}

//-----------------------------------------------------------------------------------------------
const Vec2 operator-(float uniformScale, const Vec2& vecToScale)
{
    return Vec2(uniformScale - vecToScale.x, uniformScale - vecToScale.y);
}

//-----------------------------------------------------------------------------------------------
const Vec2 operator*(float uniformScale, const Vec2& vecToScale)
{
    return Vec2(uniformScale * vecToScale.x, uniformScale * vecToScale.y);
}

//-----------------------------------------------------------------------------------------------
const Vec2 operator/( float uniformScale, const Vec2& vecToScale )
{
	return Vec2(uniformScale / vecToScale.x, uniformScale / vecToScale.y);
}

//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return x == compare.x && y == compare.y;
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return !operator==(compare);
}

