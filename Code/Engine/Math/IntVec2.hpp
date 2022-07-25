#pragma once

struct Vec2;

struct IntVec2
{
public:
	static const IntVec2 ZERO;
	static const IntVec2 ONE;
	int x = 0;
	int y = 0;

public:
	~IntVec2() {}												// destructor (do nothing)
	IntVec2() {}												// default constructor (do nothing)
	IntVec2(const IntVec2& copyFrom);							// copy constructor (from another intvec2)
	explicit IntVec2(const Vec2& vec2);				          			// from vec2 constructor (from another vec2)
	explicit IntVec2(int initialX, int initialY);		        // explicit constructor (from x, y)

	const int* data() const;
	int* data();

	int              GetLengthSquared() const;
	float            GetLength() const;
	int              GetTaxicabLength() const;
	float            GetOrientationRadians() const;
	float            GetOrientationDegrees() const;
	const IntVec2    GetRotated90Degrees() const;
	const IntVec2    GetRotatedMinus90Degrees() const;

	void             Rotate90Degrees();
	void             RotateMinus90Degrees();

	// Serialization
	void             SetFromText(const char* text);

	// Operators (const)
	bool		    operator==(const IntVec2& compare) const;		// intvec2 == intvec2
	bool		    operator!=(const IntVec2& compare) const;		// intvec2 != intvec2
	const IntVec2	operator+(const IntVec2& vecToAdd) const;		// intvec2 + intvec2
	const IntVec2	operator-(const IntVec2& vecToSubtract) const;	// intvec2 - intvec2
	const IntVec2	operator-() const;								// -intvec2, i.e. "unary negation"
	const IntVec2	operator*(int uniformScale) const;			    // intvec2 * float
	const IntVec2	operator*(const IntVec2& vecToMultiply) const;	// intvec2 * intvec2

	// Operators (self-mutating / non-const)
	void	    	operator+=(const IntVec2& vecToAdd);			// intvec2 += intvec2
	void	    	operator-=(const IntVec2& vecToSubtract);		// intvec2 -= intvec2
	void	    	operator*=(const int uniformScale);			// intvec2 *= float
	void	    	operator=(const IntVec2& copyFrom);				// intvec2 = intvec2

	// Standalone "friend" functions that are conceptually, but not actually, part of IntVec2::
	friend const IntVec2 operator*(int uniformScale, const IntVec2& vecToScale);	// float * intvec2
};

