#include "FloatRange.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

const FloatRange FloatRange::ZERO(0.0f, 0.0f);

const FloatRange FloatRange::ONE(1.0f, 1.0f);

const FloatRange FloatRange::ZERO_TO_ONE(0.0f, 1.0f);

FloatRange::FloatRange(const FloatRange& copyFrom)
	: m_min(copyFrom.m_min)
	, m_max(copyFrom.m_max)
{
}

FloatRange::FloatRange(float min, float max)
	: m_min(min)
	, m_max(max)
{
}

bool FloatRange::IsOnRange(float value) const
{
	return value >= m_min && value <= m_max;
}

bool FloatRange::IsOverlappingWith(const FloatRange& anotherRange) const
{
	return IsOnRange(anotherRange.m_min) || IsOnRange(anotherRange.m_max);
}

void FloatRange::SetFromText(const char* text)
{
	StringList strList = SplitStringOnDelimiter(text, '~');
	ASSERT_RECOVERABLE(strList.size() == 2, Stringf("Illegal text data for float range: %s", text));

	m_min = (float)atof(TrimString(strList[0]).c_str());
	m_max = (float)atof(TrimString(strList[1]).c_str());
}

FloatRange FloatRange::GetOverlapped(const FloatRange& other) const
{
	if (other.m_min > m_max)
        return FloatRange(m_max, m_max);
    if (other.m_max < m_min)
        return FloatRange(m_min, m_min);
	return FloatRange(Max(m_min, other.m_min), Min(m_max, other.m_max));
}

bool FloatRange::operator==(const FloatRange& compare) const
{
	return m_min == compare.m_min && m_max == compare.m_max;
}

bool FloatRange::operator!=(const FloatRange& compare) const
{
	return !operator==(compare);
}

void FloatRange::operator=(const FloatRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

