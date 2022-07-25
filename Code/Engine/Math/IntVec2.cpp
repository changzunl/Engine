#include "IntVec2.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "MathUtils.hpp"
#include "Vec2.hpp"
#include <math.h>

const IntVec2 IntVec2::ZERO = IntVec2(0, 0);

const IntVec2 IntVec2::ONE  = IntVec2(1, 0);

IntVec2::IntVec2(const IntVec2& copyFrom)
	: x(copyFrom.x)
	, y(copyFrom.y)
{

}

IntVec2::IntVec2(int initialX, int initialY)
	: x(initialX)
	, y(initialY)
{

}

IntVec2::IntVec2(const Vec2& vec2)
	: x(Floor(vec2.x))
	, y(Floor(vec2.y))
{

}

const int* IntVec2::data() const
{
	return (const int*)this;
}

int* IntVec2::data()
{
	return (int*)this;
}

int IntVec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float IntVec2::GetLength() const
{
	return sqrtf(static_cast<float>(GetLengthSquared()));
}

int IntVec2::GetTaxicabLength() const
{
	return abs(x) + abs(y);
}

float IntVec2::GetOrientationRadians() const
{
	float theta = atan2f(static_cast<float>(y), static_cast<float>(x));
	return theta;
}

float IntVec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(GetOrientationRadians());
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	IntVec2 vec = IntVec2(x, y);
	vec.Rotate90Degrees();
	return vec;
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	IntVec2 vec = IntVec2(x, y);
	vec.RotateMinus90Degrees();
	return vec;
}

void IntVec2::Rotate90Degrees()
{
	int oldx = x;
	x = -y;
	y = oldx;
}

void IntVec2::RotateMinus90Degrees()
{
	int oldy = y;
	y = -x;
	x = oldy;
}

void IntVec2::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() == 2, Stringf("Illegal text data for intvec2: %s", text));

	x = atoi(TrimString(strList[0]).c_str());
	y = atoi(TrimString(strList[1]).c_str());
}

const IntVec2 IntVec2::operator+(const IntVec2& vecToAdd) const
{
	return IntVec2(x + vecToAdd.x, y + vecToAdd.y);
}

const IntVec2 IntVec2::operator-(const IntVec2& vecToSubtract) const
{
	return operator+(-vecToSubtract);
}

const IntVec2 IntVec2::operator-() const
{
	return IntVec2(-x, -y);
}

void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

void IntVec2::operator*=(const int uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
}

const IntVec2 IntVec2::operator*(int uniformScale) const
{
	return IntVec2(x * uniformScale, y * uniformScale);
}

const IntVec2 IntVec2::operator*(const IntVec2& vecToMultiply) const
{
	return IntVec2(x * vecToMultiply.x, y * vecToMultiply.y);
}

void IntVec2::operator-=(const IntVec2& vecToSub)
{
	x -= vecToSub.x;
	y -= vecToSub.y;
}

void IntVec2::operator+=(const IntVec2& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}

bool IntVec2::operator!=(const IntVec2& compare) const
{
	return !operator==(compare);
}

bool IntVec2::operator==(const IntVec2& compare) const
{
	return x == compare.x && y == compare.y;
}

const IntVec2 operator*(int uniformScale, const IntVec2& vecToScale)
{
	return IntVec2(uniformScale * vecToScale.x, uniformScale * vecToScale.y);
}

