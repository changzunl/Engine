#include "Texture.hpp"

#include "Game/EngineBuildPreferences.hpp"
#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

Texture::~Texture()
{
#ifdef ENGINE_USE_SOFTWARE_RASTERIZER
	delete[](char*)m_texture;
	m_texture = nullptr;
#else
	DX_SAFE_RELEASE(m_texture);
	DX_SAFE_RELEASE(m_srv);
	DX_SAFE_RELEASE(m_rtv);
#endif
}
