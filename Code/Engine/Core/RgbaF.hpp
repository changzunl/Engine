#pragma once

struct Rgba8;

struct RgbaF
{
public:
	static const RgbaF WHITE;
	static const RgbaF BLACK;

	static RgbaF LerpColor(const RgbaF& a, const RgbaF& b, float alpha);

	~RgbaF() {}
	RgbaF() {}
	RgbaF(float r, float g, float b, float a = 1.0f);
	RgbaF(const Rgba8& rgba8);

	const float* data() const;
	float* data();

	// Serialization
	void  SetFromText(const char* text);

	// Utilities
	void  GetAsFloats(float* colorAsFloats) const;
	Rgba8 GetAsRgba8() const;
	void Normalize();

	// operators
	bool         operator==(const RgbaF& compare) const;        // RgbaF == RgbaF
	bool         operator!=(const RgbaF& compare) const;        // RgbaF != RgbaF
	void         operator*=(const RgbaF& mul);                  // RgbaF *= RgbaF
	void         operator*=(float mul);                         // RgbaF * float
	const RgbaF  operator*(const RgbaF& mul) const;             // RgbaF * RgbaF

	friend const RgbaF operator*(float mul, const RgbaF& rgba); // float * RgbaF

public:
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;
};
