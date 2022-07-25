#include "VertexBuffer.hpp"

#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

VertexBuffer::VertexBuffer(size_t size)
    : m_size(size)
{
}

VertexBuffer::~VertexBuffer()
{
#ifdef ENGINE_USE_SOFTWARE_RASTERIZER
	delete[](char*)m_buffer;
	m_buffer = nullptr;
#else
	DX_SAFE_RELEASE(m_buffer);
#endif
}

unsigned int VertexBuffer::GetStride() const
{
    return m_stride;
}

