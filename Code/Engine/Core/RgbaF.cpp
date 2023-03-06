#include "Engine/Core/RgbaF.hpp"

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "ErrorWarningAssert.hpp"
#include "StringUtils.hpp"
#include "stdlib.h"

const RgbaF RgbaF::WHITE = RgbaF(1.0f, 1.0f, 1.0f, 1.0f);
const RgbaF RgbaF::BLACK = RgbaF(0.0f, 0.0f, 0.0f, 1.0f);

RgbaF RgbaF::LerpColor(const RgbaF& a, const RgbaF& b, float alpha)
{
	RgbaF result;
	result.r = Lerp(a.r, b.r, alpha);
	result.g = Lerp(a.g, b.g, alpha);
	result.b = Lerp(a.b, b.b, alpha);
	result.a = Lerp(a.a, b.a, alpha);

	return result;
}

RgbaF::RgbaF(float r, float g, float b, float a)
	: r(r)
	, g(g)
	, b(b)
	, a(a)
{
}

RgbaF::RgbaF(const Rgba8& rgba8)
{
	rgba8.GetAsFloats((float*) this);
}

const float* RgbaF::data() const
{
	return (const float*)this;
}

float* RgbaF::data()
{
	return (float*)this;
}

void RgbaF::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() >= 3 && strList.size() <= 4, Stringf("Illegal text data for rgbaf: %s", text));
	
	r = (float) atof(TrimString(strList[0]).c_str());
	g = (float) atof(TrimString(strList[1]).c_str());
	b = (float) atof(TrimString(strList[2]).c_str());
	if (strList.size() == 4)
	{
		a = (float) atof(TrimString(strList[3]).c_str());
	}
}

void RgbaF::GetAsFloats(float* colorAsFloats) const
{
	colorAsFloats[0] = r;
	colorAsFloats[1] = g;
	colorAsFloats[2] = b;
	colorAsFloats[3] = a;
}

Rgba8 RgbaF::GetAsRgba8() const
{
	RgbaF f = *this;
	f.Normalize();

	return Rgba8(DenormalizeByte(f.r), DenormalizeByte(f.g), DenormalizeByte(f.b), DenormalizeByte(f.a));
}

void RgbaF::Normalize()
{
	Clamp(r, 0.0f, 1.0f);
	Clamp(g, 0.0f, 1.0f);
	Clamp(b, 0.0f, 1.0f);
	Clamp(a, 0.0f, 1.0f);
}

void RgbaF::operator*=(float mul)
{
	r *= mul;
	g *= mul;
	b *= mul;
	a *= mul;
}

void RgbaF::operator*=(const RgbaF& mul)
{
	r *= mul.r;
	g *= mul.g;
	b *= mul.b;
	a *= mul.a;
}

const RgbaF RgbaF::operator*(const RgbaF& mul) const
{
	return RgbaF(r * mul.r, g * mul.g, b * mul.b, a * mul.a);
}

bool RgbaF::operator!=(const RgbaF& compare) const
{
	return !operator==(compare);
}

bool RgbaF::operator==(const RgbaF& compare) const
{
	return r == compare.r && g == compare.g && b == compare.b && a == compare.a;
}

const RgbaF operator*(float mul, const RgbaF& rgba)
{
	return RgbaF(rgba.r * mul, rgba.g * mul, rgba.b * mul, rgba.a * mul);
}
