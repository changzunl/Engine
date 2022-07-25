#pragma once

#include "SpriteDefinition.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>

class Texture;

class SpriteSheet
{
public:
	explicit SpriteSheet(Texture& texture, const IntVec2& simpleGridLayout);
	SpriteSheet(const SpriteSheet& copyFrom);
	~SpriteSheet();
	
	Texture&                GetTexture() const                                             { return m_texture; }
	int                     GetNumSprites() const;
	const SpriteDefinition& GetSpriteDef(int spriteIndex) const;
	void                    GetSpriteUVs(AABB2& out_uvAtMinMaxs, int spriteIndex) const;             
	const AABB2&            GetSpriteUVs(int spriteIndex) const;

protected:
	Texture&                         m_texture;
	std::vector<SpriteDefinition>    m_spriteDefs;
};

