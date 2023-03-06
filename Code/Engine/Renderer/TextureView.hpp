#pragma once

class Renderer;
class Texture;

struct ID3D11Texture2D;
struct ID3D11View;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class TextureView
{
    friend class Renderer;
    friend class Texture;

private:
    TextureView();
    ~TextureView();

    TextureView(const TextureView&) = delete;

public:

private:
    ID3D11Texture2D* m_resourceHandle = nullptr;
    union
    {
        ID3D11ShaderResourceView* m_srv;
        ID3D11RenderTargetView* m_rtv;
        ID3D11DepthStencilView* m_dsv;
    } m_views;
};
