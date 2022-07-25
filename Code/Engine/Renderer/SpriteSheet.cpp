#include "SpriteSheet.hpp"

#include "Texture.hpp"

SpriteSheet::SpriteSheet(Texture& texture, const IntVec2& simpleGridLayout)
	: m_texture(texture)
{
	const int& rowLength = simpleGridLayout.x;
	const int& columnLength = simpleGridLayout.y;
	const float uLength = 1.0f / (float) rowLength;
	const float vLength = 1.0f / (float) columnLength;
	const float pixelWidth = 1.0f / (float)texture.GetDimensions().x;
	const float pixelHeight = 1.0f / (float)texture.GetDimensions().y;
	const Vec2 pixelShrink = Vec2(pixelWidth * 0.01f * 2, pixelHeight * 0.01f * 2);

	m_spriteDefs.reserve(rowLength * columnLength);
	for (int y = 0; y < columnLength; y++)
	{
		for (int x = 0; x < rowLength; x++)
		{
			int spriteIndex = x + rowLength * y;
			AABB2 uvAtMinMaxs = AABB2((float)x * uLength, 1.0f - ((float)(y + 1) * vLength), (float)(x + 1) * uLength, 1.0f - ((float)y * vLength));
			uvAtMinMaxs.SetDimensions(uvAtMinMaxs.GetDimensions() - pixelShrink);
			m_spriteDefs.emplace_back(this[0], spriteIndex, uvAtMinMaxs);
		}
	}
}

SpriteSheet::SpriteSheet(const SpriteSheet& copyFrom)
	: m_texture(copyFrom.m_texture)
	, m_spriteDefs(copyFrom.m_spriteDefs)
{
}

SpriteSheet::~SpriteSheet()
{
}

int SpriteSheet::GetNumSprites() const
{
	return (int)m_spriteDefs.size();
}

const SpriteDefinition& SpriteSheet::GetSpriteDef(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex];
}

void SpriteSheet::GetSpriteUVs(AABB2& out_uvAtMinMaxs, int spriteIndex) const
{
	m_spriteDefs[spriteIndex].GetUVs(out_uvAtMinMaxs);
}

const AABB2& SpriteSheet::GetSpriteUVs(int spriteIndex) const
{
	return m_spriteDefs[spriteIndex].GetUVs();
}

