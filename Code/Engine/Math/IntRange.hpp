#pragma once

struct IntRange
{
public:
	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;
	int m_min = 0;
	int m_max = 0;

public:
	~IntRange() {}
	IntRange() {}
	IntRange(const IntRange& copyFrom);
	explicit IntRange(int min, int max);

	// utilities
	bool    IsOnRange(int value) const;
	bool    IsOverlappingWith(const IntRange& anotherRange) const;

	// Operators
	bool    operator==(const IntRange& compare) const;
	bool    operator!=(const IntRange& compare) const;
	void    operator=(const IntRange& copyFrom);
};

