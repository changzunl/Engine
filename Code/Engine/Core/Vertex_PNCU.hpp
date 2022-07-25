#pragma once
#include "Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

struct Vertex_PNCU
{

public:
	Vec3 m_position;
	Vec3 m_normal;
	Rgba8 m_color;
	Vec2 m_uvTexCoords;

public:
	~Vertex_PNCU() {}
	Vertex_PNCU () {}
	explicit Vertex_PNCU(const Vec3& position, const Vec3& normal, const Rgba8& tint, const Vec2& uvTexCoords);

};

