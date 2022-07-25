#include "IntRange.hpp"

const IntRange IntRange::ZERO(0, 0);

const IntRange IntRange::ONE(1, 1);

const IntRange IntRange::ZERO_TO_ONE(0, 1);

IntRange::IntRange(const IntRange& copyFrom)
	: m_min(copyFrom.m_min)
	, m_max(copyFrom.m_max)
{
}

IntRange::IntRange(int min, int max)
	: m_min(min)
	, m_max(max)
{
}

bool IntRange::IsOnRange(int value) const
{
	return value >= m_min && value <= m_max;
}

bool IntRange::IsOverlappingWith(const IntRange& anotherRange) const
{
	return IsOnRange(anotherRange.m_min) || IsOnRange(anotherRange.m_max);
}

bool IntRange::operator==(const IntRange& compare) const
{
	return m_min == compare.m_min && m_max == compare.m_max;
}

bool IntRange::operator!=(const IntRange& compare) const
{
	return !operator==(compare);
}

void IntRange::operator=(const IntRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

