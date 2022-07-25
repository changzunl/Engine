#include "Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "ErrorWarningAssert.hpp"
#include "StringUtils.hpp"
#include "stdlib.h"

const Rgba8 Rgba8::WHITE = Rgba8(255, 255, 255, 255);
const Rgba8 Rgba8::BLACK = Rgba8(  0,   0,   0, 255);
const Rgba8 Rgba8::RED   = Rgba8(255,   0,   0, 255);
const Rgba8 Rgba8::GREEN = Rgba8(  0, 255,   0, 255);
const Rgba8 Rgba8::BLUE  = Rgba8(  0,   0, 255, 255);

Rgba8::Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	: r(r)
	, g(g)
	, b(b)
	, a(a)
{
}

const unsigned char* Rgba8::data() const
{
	return (const unsigned char*)this;
}

unsigned char* Rgba8::data()
{
	return (unsigned char*)this;
}

void Rgba8::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, ',');
	ASSERT_RECOVERABLE(strList.size() >= 3 && strList.size() <= 4, Stringf("Illegal text data for rgba8: %s", text));
	
	r = (unsigned char) atoi(TrimString(strList[0]).c_str());
	g = (unsigned char) atoi(TrimString(strList[1]).c_str());
	b = (unsigned char) atoi(TrimString(strList[2]).c_str());
	if (strList.size() == 4)
	{
		a = (unsigned char) atoi(TrimString(strList[3]).c_str());
	}
}

void Rgba8::GetAsFloats(float* colorAsFloats) const
{
	colorAsFloats[0] = Clamp((static_cast<float>(r) + 0.5f) / 256.0f, 0.0f, 1.0f);
	colorAsFloats[1] = Clamp((static_cast<float>(g) + 0.5f) / 256.0f, 0.0f, 1.0f);
	colorAsFloats[2] = Clamp((static_cast<float>(b) + 0.5f) / 256.0f, 0.0f, 1.0f);
	colorAsFloats[3] = Clamp((static_cast<float>(a) + 0.5f) / 256.0f, 0.0f, 1.0f);
}

bool Rgba8::operator!=(const Rgba8& compare) const
{
	return !operator==(compare);
}

bool Rgba8::operator==(const Rgba8& compare) const
{
	return r == compare.r && g == compare.g && b == compare.b && a == compare.a;
}

