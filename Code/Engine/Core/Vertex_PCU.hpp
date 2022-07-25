#pragma once
#include "Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

struct Vertex_PCU
{

public:
	Vec3 m_position;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

public:
	~Vertex_PCU() {}
	Vertex_PCU () {}
	explicit Vertex_PCU(const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords);

};
