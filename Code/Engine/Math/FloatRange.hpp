#pragma once

struct FloatRange
{
public:
	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;
	float m_min = 0.0f;
	float m_max = 0.0f;

public:
	~FloatRange() {}
	FloatRange() {}
	FloatRange(const FloatRange& copyFrom);
	explicit FloatRange(float min, float max);

	// utilities
	bool    IsOnRange(float value) const;
	bool    IsOverlappingWith(const FloatRange& anotherRange) const;
	void    SetFromText(const char* text);
	FloatRange GetOverlapped(const FloatRange& other) const;

	// Operators
	bool    operator==(const FloatRange& compare) const;
	bool    operator!=(const FloatRange& compare) const;
	void    operator=(const FloatRange& copyFrom);
};

