#pragma once

struct Rgba8
{
public:
	static const Rgba8 RED;
	static const Rgba8 GREEN;
    static const Rgba8 BLUE;
    static const Rgba8 WHITE;
	static const Rgba8 CYAN;
    static const Rgba8 MAGENTA;
    static const Rgba8 YELLOW;
    static const Rgba8 BLACK;

	~Rgba8() {}
	Rgba8() {}
	Rgba8(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

	const unsigned char* data() const;
	unsigned char* data();

	// Serialization
	void  SetFromText(const char* text);

	// Utilities
	void GetAsFloats(float* colorAsFloats) const;

	// operators
	bool  operator==(const Rgba8& compare) const;		// Rgba8 == Rgba8
	bool  operator!=(const Rgba8& compare) const;		// Rgba8 != Rgba8

public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;
};
