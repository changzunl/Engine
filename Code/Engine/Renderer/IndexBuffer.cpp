#include "IndexBuffer.hpp"

#include "Game/EngineBuildPreferences.hpp"
#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

IndexBuffer::IndexBuffer(size_t size)
    : m_size(size)
{
}

IndexBuffer::~IndexBuffer()
{
#ifdef ENGINE_USE_SOFTWARE_RASTERIZER
    delete[] (char*)m_buffer;
    m_buffer = nullptr;
#else
    DX_SAFE_RELEASE(m_buffer);
#endif
}

