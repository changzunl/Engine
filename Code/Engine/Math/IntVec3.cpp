#include "IntVec3.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "MathUtils.hpp"
#include "Vec2.hpp"
#include <math.h>

const IntVec3 IntVec3::ZERO = IntVec3(0, 0, 0);

const IntVec3 IntVec3::ONE  = IntVec3(1, 0, 0);

IntVec3::IntVec3(const IntVec2& vec2, int initialZ /*= 0*/)
	: x(vec2.x)
	, y(vec2.y)
	, z(initialZ)
{
}

IntVec3::IntVec3(const Vec3& vec3)
	: x(Floor(vec3.x))
	, y(Floor(vec3.y))
	, z(Floor(vec3.z))
{
}

IntVec3::IntVec3(const Vec2& vec2, float initialZ /*= 0.0f*/)
	: x(Floor(vec2.x))
	, y(Floor(vec2.y))
	, z(Floor(initialZ))
{
}

const int* IntVec3::data() const
{
	return (const int*)this;
}

int* IntVec3::data()
{
	return (int*)this;
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

float IntVec3::GetLength() const
{
	return sqrtf(static_cast<float>(GetLengthSquared()));
}

int IntVec3::GetTaxicabLength() const
{
	return abs(x) + abs(y) + abs(z);
}

void IntVec3::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() == 3, Stringf("Illegal text data for IntVec3: %s", text));

	x = atoi(TrimString(strList[0]).c_str());
	y = atoi(TrimString(strList[1]).c_str());
	y = atoi(TrimString(strList[2]).c_str());
}

const IntVec3 IntVec3::operator+(const IntVec3& vecToAdd) const
{
	return IntVec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

const IntVec3 IntVec3::operator-(const IntVec3& vecToSubtract) const
{
	return operator+(-vecToSubtract);
}

const IntVec3 IntVec3::operator-() const
{
	return IntVec3(-x, -y, -z);
}

void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

void IntVec3::operator*=(const int uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}

const IntVec3 IntVec3::operator*(int uniformScale) const
{
	return IntVec3(x * uniformScale, y * uniformScale, z * uniformScale);
}

const IntVec3 IntVec3::operator*(const IntVec3& vecToMultiply) const
{
	return IntVec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}

void IntVec3::operator-=(const IntVec3& vecToSub)
{
	x -= vecToSub.x;
	y -= vecToSub.y;
	z -= vecToSub.z;
}

void IntVec3::operator+=(const IntVec3& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}

bool IntVec3::operator!=(const IntVec3& compare) const
{
	return !operator==(compare);
}

bool IntVec3::operator==(const IntVec3& compare) const
{
	return x == compare.x && y == compare.y && z == compare.z;
}

const IntVec3 operator*(int uniformScale, const IntVec3& vecToScale)
{
	return IntVec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}

