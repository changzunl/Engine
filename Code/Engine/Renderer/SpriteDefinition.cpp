#include "SpriteDefinition.hpp"

#include "SpriteSheet.hpp"
#include "Texture.hpp"

SpriteDefinition::SpriteDefinition(const SpriteSheet& spriteSheet, int spriteIndex, const AABB2& uvAtMinMaxs)
	: m_spriteSheet(spriteSheet)
	, m_spriteIndex(spriteIndex)
	, m_uvAtMinMaxs(uvAtMinMaxs)
{
}

Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}

void SpriteDefinition::GetUVs(AABB2& out_uvAtMinMaxs) const
{
	out_uvAtMinMaxs.m_mins = m_uvAtMinMaxs.m_mins;
	out_uvAtMinMaxs.m_maxs = m_uvAtMinMaxs.m_maxs;
}

float SpriteDefinition::GetAspect() const
{
	Vec2    uvDimensions    = m_uvAtMinMaxs.GetDimensions();
	IntVec2 pixelDimensions = GetTexture().GetDimensions();

	return (uvDimensions.x * (float)pixelDimensions.x) / (uvDimensions.y * (float)pixelDimensions.y);
}

