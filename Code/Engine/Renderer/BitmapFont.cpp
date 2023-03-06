#include "BitmapFont.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"

#include <vector>

Vec2 BTIMAP_FONT_OVERSIZE = Vec2(1.0f, 1.0f);

BitmapFont::BitmapFont(const char* fontFilePathNameWithNoExtension, Texture& fontTexture)
	: m_fontFilePathNameWithNoExtension(fontFilePathNameWithNoExtension)
	, m_fontGlyphsSpriteSheet(fontTexture, IntVec2(16, 16))
{
}

Texture& BitmapFont::GetTexture() const
{
	return m_fontGlyphsSpriteSheet.GetTexture();
}

void BitmapFont::AddVertsForText2D(
	std::vector<Vertex_PCU>&    verts, 
	const Vec2&                 textMins, 
	float                       cellHeight, 
	const std::string&          text, 
	const Rgba8&                tint, 
	float                       cellAspect) const
{
	Vec2 textMinsPos = textMins;

	for (const unsigned char& ch : text)
	{
		if (ch == 0) break;

		AABB2 aabb = AABB2(0, 0, cellAspect * GetGlyphAspect(ch) * cellHeight, cellHeight);
		float width = aabb.m_maxs.x;
		aabb.Translate(textMinsPos);
		aabb.SetDimensions(aabb.GetDimensions() * BTIMAP_FONT_OVERSIZE);
		textMinsPos += Vec2(width, 0.0f);
		AddVertsForAABB2D(verts, aabb, tint, m_fontGlyphsSpriteSheet.GetSpriteUVs(ch));
	}
}

void BitmapFont::AddVertsForTextInBox2D(
	std::vector<Vertex_PCU>&    verts, 
	const AABB2&                box, 
	float                       cellHeight, 
	const std::string&          text, 
	const Rgba8&                tint, 
	float                       cellAspect, 
	const Vec2&                 alignment, 
	TextDrawMode                mode, 
	int                         maxGlyphsToDraw) const
{
	// data preparation, split text, calculate line width
	StringList strList = SplitStringOnDelimiter(text, '\n');
	std::vector<float> lineWidths = std::vector<float>(strList.size());
	Vec2 textDims = Vec2(0.0f, cellHeight * strList.size());
	for (int idx = 0; idx < strList.size(); idx++)
	{
		float lineWidth = lineWidths[idx] = GetTextWidth(cellHeight, strList[idx], cellAspect);
		if (lineWidth > textDims.x)
		{
			textDims.x = lineWidth;
		}
	}

	// calculate scale if is shrink mode
	float scale = 1.0f;
	if (mode == TextDrawMode::SHRINK_TO_FIT)
	{
		Vec2 boxDims = box.GetDimensions();
		if (textDims.y > boxDims.y)
		{
			scale *= boxDims.y / textDims.y;
		}
		if (textDims.x * scale > boxDims.x)
		{
			scale *= boxDims.x / (textDims.x * scale);
		}
	}

	// add verts in box
	AABB2 textBox = AABB2(0.0f, 0.0f, textDims.x * scale, textDims.y * scale);
	textBox.AlignToBox(box, alignment);
	for (int idx = 0; idx < strList.size(); idx++)
	{
		int lineIdx = (int)strList.size() - idx - 1;
		AABB2 lineBox = AABB2(0.0f, cellHeight * idx * scale, lineWidths[lineIdx] * scale, cellHeight * (idx + 1) * scale);
		lineBox.AlignToBoxHorizontal(textBox, alignment.x);
		lineBox.Translate(Vec2(0.0f, textBox.m_mins.y));
		if (maxGlyphsToDraw >= strList[lineIdx].size())
		{
			maxGlyphsToDraw -= (int)strList[lineIdx].size();
			AddVertsForText2D(verts, lineBox.m_mins, cellHeight * scale, strList[lineIdx], tint, cellAspect);
		}
		else
		{
			std::string subText = strList[lineIdx].substr(0, maxGlyphsToDraw);
			AddVertsForText2D(verts, lineBox.m_mins, cellHeight * scale, subText, tint, cellAspect);
			break;
		}
	}
}

float BitmapFont::GetTextWidth(float cellHeight, const std::string& text, float cellAspect /*= 1.f*/) const
{
	float textWidth = 0.0f;
	for (const unsigned char& ch : text)
	{
		if (ch == 0) break;

		textWidth += GetGlyphAspect(ch);
	}
	textWidth *= cellHeight * cellAspect;
	return textWidth;
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	UNUSED(glyphUnicode);

	return 1.0f;
}

