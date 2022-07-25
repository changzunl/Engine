#include "Vertex_PCU.hpp"


Vertex_PCU::Vertex_PCU(const Vec3& position, const Rgba8& tint, const Vec2& uvTexCoords)
	: m_position(position)
	, m_color(tint)
	, m_uvTexCoords(uvTexCoords)
{
}
