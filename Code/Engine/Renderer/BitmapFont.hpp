#pragma once

#include "SpriteSheet.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>

struct Vertex_PCU;
struct AABB2;

enum class TextDrawMode
{
	SHRINK_TO_FIT,
	OVERRUN,
};

class BitmapFont
{
	friend class Renderer; // Only the Renderer can create new BitmapFont objects!

private:
	BitmapFont(const char* fontFilePathNameWithNoExtension, Texture& fontTexture);

public:
	Texture&    GetTexture() const;
	void        AddVertsForText2D(std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f) const;
	void        AddVertsForTextInBox2D(std::vector<Vertex_PCU>& vertexArray, const AABB2& box, float cellHeight, const std::string& text, const Rgba8& tint = Rgba8::WHITE, float cellAspect = 1.f, const Vec2& alignment = Vec2(.5f, .5f), TextDrawMode mode = TextDrawMode::SHRINK_TO_FIT, int maxGlyphsToDraw = 99999999) const;
	float       GetTextWidth(float cellHeight, const std::string& text, float cellAspect = 1.f) const;

protected:
	float       GetGlyphAspect(int glyphUnicode) const; // For now this will always return 1.0f!!!

protected:
	std::string	m_fontFilePathNameWithNoExtension;
	SpriteSheet	m_fontGlyphsSpriteSheet;
};


