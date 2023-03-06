#pragma once

#include "Engine/Math/IntVec2.hpp"
#include <string>

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class TextureView;

enum ETextureBindFlagBit : unsigned int
{
    TEXTURE_BIND_SHADER_RESOURCE_BIT = (1 << 0),
    TEXTURE_BIND_RENDER_TARGET_BIT = (1 << 1),
    TEXTURE_BIND_DEPTH_STENCIL_BIT = (1 << 2),
};

using ETextureBindFlags = unsigned int;

enum class ETextureFormat
{
	R8G8B8A8_UNORM,
	D24_UNORM_S8_UINT,
	R24G8_TYPELESS,
	R24_UNORM_X8_TYPELESS,
    R32_FLOAT,
};


enum class EMemoryHint
{
	STATIC,          // d3d11 default
	GPU,             // d3d11 immutable
	DYNAMIC,         // d3d11 dynamic
	// STAGING,         // d3d11 staging
};


int GetD3DConstant(ETextureFormat format);
int GetD3DConstant(EMemoryHint hint);
int GetD3DConstant(ETextureBindFlagBit bit);
int GetD3DFlags(ETextureBindFlags flags);


struct TextureCreateInfo
{
    std::string name;
    IntVec2 dimensions;
    size_t initDataLength = 0;
    const void* initData = nullptr;
    ETextureFormat format = ETextureFormat::R8G8B8A8_UNORM;
    ETextureBindFlags bindFlags = TEXTURE_BIND_SHADER_RESOURCE_BIT;
    EMemoryHint memoryHint = EMemoryHint::STATIC;

    void SetFormat(bool isR32, bool isDepth, bool isTarget, bool isDynamic);
};


struct TextureViewInfo
{
	ETextureBindFlagBit type;
};


class Texture
{
	friend class Renderer; // Only the Renderer can create new Texture objects!

private:
	Texture() {} // can't instantiate directly; must ask Renderer to do it for you
	Texture(const Texture&) = delete;
	Texture(Texture&&) = delete;
	~Texture();

public:
	IntVec2				  GetDimensions() const                                { return m_info.dimensions; }
	std::string const&    GetImageFilePath() const                             { return m_info.name; }

	void operator=(const Texture&) = delete;
	void operator=(Texture&&) = delete;

protected:
	TextureCreateInfo         m_info;

	ID3D11Texture2D*          m_texture    = nullptr;
	ID3D11ShaderResourceView* m_srv        = nullptr;
	ID3D11RenderTargetView*   m_rtv        = nullptr;
	ID3D11DepthStencilView*   m_dsv        = nullptr;
};

