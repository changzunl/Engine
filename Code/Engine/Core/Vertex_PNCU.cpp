#include "Vertex_PNCU.hpp"


Vertex_PNCU::Vertex_PNCU(const Vec3& position, const Vec3& normal, const Rgba8& tint, const Vec2& uvTexCoords)
	: m_position(position)
	, m_normal(normal)
	, m_color(tint)
	, m_uvTexCoords(uvTexCoords)
{
}
