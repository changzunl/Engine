#include "Texture.hpp"

#include "Game/EngineBuildPreferences.hpp"
#include <atomic>

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


void TextureCreateInfo::SetFormat(bool isR32, bool isDepth, bool isTarget, bool isDynamic)
{
    static std::atomic_int name_counter;

    format = isR32 ? ETextureFormat::R32_FLOAT : isDepth ? ETextureFormat::R24G8_TYPELESS : ETextureFormat::R8G8B8A8_UNORM;
    memoryHint = (isTarget || isDepth) ? EMemoryHint::STATIC : isDynamic ? EMemoryHint::DYNAMIC : EMemoryHint::GPU;
    bindFlags = TEXTURE_BIND_SHADER_RESOURCE_BIT;
    if (isTarget)
        bindFlags |= TEXTURE_BIND_RENDER_TARGET_BIT;
    if (isDepth)
        bindFlags |= TEXTURE_BIND_DEPTH_STENCIL_BIT;

    if (name.empty())
    {
        name = (isDepth ? "DEPTH_" : "TEXTURE_") + std::to_string(name_counter++);
    }
}
