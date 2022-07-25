#pragma once

#include "Engine/Math/AABB2.hpp"

class SpriteSheet;
class Texture;

class SpriteDefinition
{
public:
	explicit SpriteDefinition(const SpriteSheet& spriteSheet, int spriteIndex, const AABB2& uvAtMinMaxs);
	
	const SpriteSheet&    GetSpriteSheet() const                  { return m_spriteSheet; }
	Texture&              GetTexture() const;
	void                  GetUVs(AABB2& out_uvAtMinMaxs) const;
	const AABB2&          GetUVs() const                          { return m_uvAtMinMaxs; }
	float                 GetAspect() const;

protected:
	const SpriteSheet&    m_spriteSheet;
	int                   m_spriteIndex = -1;
	AABB2                 m_uvAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f);
};

