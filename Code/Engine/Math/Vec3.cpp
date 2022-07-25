#include "Vec3.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Vec2.hpp"
#include "MathUtils.hpp"
#include <math.h>

const Vec3 Vec3::ZERO = Vec3();
const Vec3 Vec3::ONE  = Vec3(1.0f, 0.0f, 0.0f);

//-----------------------------------------------------------------------------------------------
Vec3::Vec3(const Vec3& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
{
}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3(float initialX, float initialY, float initialZ)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
{
}

Vec3::Vec3(const Vec2& fromVec, float initialZ)
	: x(fromVec.x)
	, y(fromVec.y)
	, z(initialZ)
{
}

const float* Vec3::data() const
{
	return (const float*)this;
}

float* Vec3::data()
{
	return (float*)this;
}

float Vec3::GetLength() const
{
	return sqrtf(GetLengthSquared());
}

float Vec3::GetLengthXY() const
{
	return sqrtf(GetLengthXYSquared());
}

float Vec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

float Vec3::GetLengthXYSquared() const
{
	return x * x + y * y;
}

float Vec3::GetAngleAboutZRadians() const
{
	return atan2f(y, x);
}

float Vec3::GetAngleAboutZDegrees() const
{
	return ConvertRadiansToDegrees(GetAngleAboutZRadians());
}

const Vec3 Vec3::GetRotatedAboutZRadians(float radians) const
{
	float c = cosf(radians);
	float s = sinf(radians);
	float newX = x * c + y * -s;
	float newY = x * s + y *  c;

	return Vec3(newX, newY, z);
}

const Vec3 Vec3::GetRotatedAboutZDegrees(float degrees) const
{
	return GetRotatedAboutZRadians(ConvertDegreesToRadians(degrees));
}

const Vec3 Vec3::GetClamped(float maxLength) const
{
	if (GetLengthSquared() <= maxLength * maxLength)
	{
		return Vec3(x, y, z);
	}
	else
	{
		float scale = maxLength / GetLength();
		return Vec3(x * scale, y * scale, z * scale);
	}
}

const Vec3 Vec3::GetNormalized() const
{
	Vec3 result = *this;
	result.NormalizeAndGetPreviousLength();
	return result;
}

float Vec3::Dot(const Vec3& b) const
{
	return x * b.x + y * b.y + z * b.z;
}

float Vec3::Dot2D(const Vec3& b) const
{
	return x * b.x + y * b.y;
}

const Vec3 Vec3::Cross(const Vec3& b) const
{
	return Vec3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
}

float Vec3::Cross2D(const Vec3& b) const
{
	return x * b.y - y * b.x;
}

void Vec3::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() == 3, Stringf("Illegal text data for vec3: %s", text));

	x = (float) atof(TrimString(strList[0]).c_str());
	y = (float) atof(TrimString(strList[1]).c_str());
	z = (float) atof(TrimString(strList[2]).c_str());
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + (const Vec3& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-(const Vec3& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}


const Vec3 Vec3::operator-(float valToSubtract) const
{
	return Vec3(x - valToSubtract, y - valToSubtract, z - valToSubtract);
}

void Vec3::operator/=(const Vec3& vecToDiv)
{
	x /= vecToDiv.x;
	y /= vecToDiv.y;
	z /= vecToDiv.z;
}

void Vec3::operator*=(const Vec3& vecToMul)
{
	x *= vecToMul.x;
	y *= vecToMul.y;
	z *= vecToMul.z;
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*(const Vec3& vecToMultiply) const
{
	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}

const float Vec3::NormalizeAndGetPreviousLength()
{
	if (x == 0 && y == 0 && z == 0)
	{
		x = 1;
		return 0;
	}

	float length = GetLength();
	float scale = 1.f / length;
	x *= scale;
	y *= scale;
	z *= scale;
	return length;
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/(float inverseScale) const
{
	return Vec3(x / inverseScale, y / inverseScale, z / inverseScale);
}


const Vec3 Vec3::operator/(const Vec3& vecToDiv) const
{
	return Vec3(x / vecToDiv.x, y / vecToDiv.y, z / vecToDiv.z);
}

void Vec3::operator-=(float valToSubtract)
{
	x -= valToSubtract;
	y -= valToSubtract;
	z -= valToSubtract;
}

void Vec3::operator+=(float valToAdd)
{
	x += valToAdd;
	y += valToAdd;
	z += valToAdd;
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator+=(const Vec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


const Vec3 Vec3::operator+(float valToAdd) const
{
	return Vec3(x + valToAdd, y + valToAdd,	z + valToAdd);
}

//-----------------------------------------------------------------------------------------------
void Vec3::operator-=(const Vec3& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=(const Vec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*(float uniformScale, const Vec3& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==(const Vec3& compare) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=(const Vec3& compare) const
{
	return !operator==(compare);
}

