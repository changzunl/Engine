#include "Renderer.hpp"

#ifndef ENGINE_USE_SOFTWARE_RASTERIZER

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Window//Window.hpp"
#include "Engine/Window//WindowConfig.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/VertexFormat.hpp"
#include <ThirdParty/stb/stb_image.h>

#include <atomic>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#undef OPAQUE

#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

#include <dxgi1_2.h>

#ifdef ENGINE_DEBUG_RENDER
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib" )
#endif

constexpr float BUFFER_LOAD_FACTOR = 1.33333333f;

Renderer::Renderer(const RendererConfig& theConfig)
	: m_theConfig(theConfig)
{
}

Renderer::~Renderer()
{

}

void Renderer::Startup()
{
	CreateRenderContext();
	CreateRenderState();
	InitializeRenderState();

	DebugRenderConfig config;
	config.m_renderer = this;
	DebugRenderSystemStartup(config);
}

void Renderer::CreateRenderContext()
{
#ifdef ENGINE_DEBUG_RENDER
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB) ::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);
#endif

	// DX11 init
	HRESULT result = {};

	// DX11 device initialize

    // result = ::D3D11CreateDeviceAndSwapChain(nullptr, paramDriverType, 0, paramFlags, 0, 0, paramSDKVersion, &paramSwapChainDesc, &swapChain, &m_device, &paramFeatureLevel, &m_deviceContext);
	// ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ::D3D11CreateDeviceAndSwapChain");

    UINT paramSDKVersion = D3D11_SDK_VERSION;
	D3D_DRIVER_TYPE paramDriverType = D3D_DRIVER_TYPE_HARDWARE;
	UINT paramFlags = 0;
	D3D_FEATURE_LEVEL paramFeatureLevel = D3D_FEATURE_LEVEL_12_1;
#ifdef ENGINE_DEBUG_RENDER
	paramFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    result = ::D3D11CreateDevice(nullptr, paramDriverType, 0, paramFlags, 0, 0, paramSDKVersion, &m_device, &paramFeatureLevel, &m_deviceContext);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ::D3D11CreateDevice");

	// default swap chain
    DXGI_SWAP_CHAIN_DESC paramSwapChainDesc = {};
	paramSwapChainDesc.BufferDesc.Width = m_theConfig.m_window->GetClientDimensions().x;
	paramSwapChainDesc.BufferDesc.Height = m_theConfig.m_window->GetClientDimensions().y;
	paramSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	paramSwapChainDesc.SampleDesc.Count = 1;
	paramSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	paramSwapChainDesc.BufferCount = 2;
	paramSwapChainDesc.OutputWindow = HWND(m_theConfig.m_window->GetHwnd());
	paramSwapChainDesc.Windowed = true;
	paramSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	m_mainScreen.screenBuffer = new Texture();
	m_mainScreen.screenBuffer->m_info.name = "SCREEN_BUFFER";
	m_mainScreen.screenBuffer->m_info.dimensions = m_theConfig.m_window->GetClientDimensions();

	IDXGIDevice* dxgiDevice;
	result = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to cast ID3D11Device -> IDXGIDevice");

	IDXGIAdapter* dxgiAdapter;
	result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to cast ID3D11Device -> IDXGIAdapter");

	IDXGIFactory* dxgiFactory;
	result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to cast ID3D11Device -> IDXGIFactory");

	m_mainScreen.window = m_theConfig.m_window;
	result = dxgiFactory->CreateSwapChain(m_device, &paramSwapChainDesc, &m_mainScreen.swapChain);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call IDXGIFactory::CreateSwapChain");

	DX_SAFE_RELEASE(dxgiFactory);
	DX_SAFE_RELEASE(dxgiAdapter);
	DX_SAFE_RELEASE(dxgiDevice);

	m_mainScreen.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_mainScreen.screenBuffer->m_texture);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call IDXGISwapChain::GetBuffer");
	m_device->CreateRenderTargetView(m_mainScreen.screenBuffer->m_texture, nullptr, &m_mainScreen.screenBuffer->m_rtv);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateRenderTargetView");
}

void Renderer::CreateRenderState()
{

	// init viewport
	SetViewport(AABB2::ZERO_TO_ONE);

	// create depth stencil state
	CreateDepthState();

	// create blend state
	m_blendState.m_blendMode = (BlendMode)-1;
	CreateBlendState();

	// create sampler state
	m_samplerState.m_samplerMode = (SamplerMode)-1;
	CreateSamplerState();

	// create shader cbo
	m_modelCBO  = CreateConstantBuffer(sizeof(ModelConstants));
	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));
	UpdateLights();

	// create default vbo
	m_immediateVBO = CreateVertexBuffer(m_theConfig.m_initialVBOSize);

	// create default texture
	m_defaultTexture = CreateTextureFromImage(Image::WHITE_PIXEL);

	// create default shader
	m_defaultVF_PCU = VertexFormat::GetDefaultFormat_Vertex_PCU();
	m_defaultVF_PNCU = VertexFormat::GetDefaultFormat_Vertex_PNCU();
	std::string shaderSource = SHADER_SOURCE_EMBEDDED;
	m_defaultShader = CreateShader("Default", shaderSource, m_defaultVF_PCU);
	m_loadedShaders.clear();
}

void Renderer::CreateDepthState()
{
	HRESULT result = {};

	m_depthStencilState.m_depthStencilTexture = new Texture();
	m_depthStencilState.m_depthStencilTexture->m_info.dimensions = m_theConfig.m_window->GetClientDimensions();
	m_depthStencilState.m_depthStencilTexture->m_info.name = "TEXTURE_DEPTH_STENCIL";

	D3D11_TEXTURE2D_DESC pTextureDesc = {};
	pTextureDesc.Width = m_theConfig.m_window->GetClientDimensions().x;
	pTextureDesc.Height = m_theConfig.m_window->GetClientDimensions().y;
	pTextureDesc.MipLevels = 1;
	pTextureDesc.ArraySize = 1;
	pTextureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	pTextureDesc.SampleDesc.Count = 1;
	pTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	pTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	result = m_device->CreateTexture2D(&pTextureDesc, nullptr, &m_depthStencilState.m_depthStencilTexture->m_texture);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateTexture2D");

	D3D11_DEPTH_STENCIL_VIEW_DESC pDepthStencilDesc = {};
	pDepthStencilDesc.Flags = 0;
	pDepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	pDepthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	pDepthStencilDesc.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(m_depthStencilState.m_depthStencilTexture->m_texture, &pDepthStencilDesc, &m_depthStencilState.m_depthStencilView);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateDepthStencilView");

	D3D11_SHADER_RESOURCE_VIEW_DESC pShaderResourceViewDesc = {};
	pShaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	pShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	pShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	pShaderResourceViewDesc.Texture2D.MipLevels = (UINT)-1;

	result = m_device->CreateShaderResourceView(m_depthStencilState.m_depthStencilTexture->m_texture, &pShaderResourceViewDesc, &m_depthStencilState.m_depthStencilTexture->m_srv);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateShaderResourceView");
	SetDebugName(m_depthStencilState.m_depthStencilTexture->m_texture, m_depthStencilState.m_depthStencilTexture->m_info.name.c_str());

	SetDepthTarget(nullptr);
}

void Renderer::CreateBlendState()
{
	HRESULT result = {};

	D3D11_BLEND_DESC blendStateDescription = {};

	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = m_device->CreateBlendState(&blendStateDescription, &m_blendState.m_blendStates[(int)BlendMode::OPAQUE]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateBlendState");

	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = m_device->CreateBlendState(&blendStateDescription, &m_blendState.m_blendStates[(int)BlendMode::ALPHA]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateBlendState");

	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = m_device->CreateBlendState(&blendStateDescription, &m_blendState.m_blendStates[(int)BlendMode::ADDITIVE]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateBlendState");
}

void Renderer::CreateSamplerState()
{
	HRESULT result = {};
	D3D11_SAMPLER_DESC pSamplerDesc = {};

	pSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	pSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	pSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	pSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	pSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_device->CreateSamplerState(&pSamplerDesc, &m_samplerState.m_samplerStates[(int)SamplerMode::POINTCLAMP]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateSamplerState");

	pSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	pSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	pSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	pSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_device->CreateSamplerState(&pSamplerDesc, &m_samplerState.m_samplerStates[(int)SamplerMode::POINTWRAP]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateSamplerState");

	pSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	pSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	pSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	pSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	pSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	pSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_device->CreateSamplerState(&pSamplerDesc, &m_samplerState.m_samplerStates[(int)SamplerMode::BILINEARCLAMP]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateSamplerState");

	pSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	pSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	pSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	pSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	pSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = m_device->CreateSamplerState(&pSamplerDesc, &m_samplerState.m_samplerStates[(int)SamplerMode::BILINEARWRAP]);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateSamplerState");
}

void Renderer::InitializeRenderState()
{
	m_blendState.SetBlendMode(BlendMode::ALPHA);
	m_samplerState.SetSamplerMode(SamplerMode::POINTCLAMP);
	BindShader(nullptr);
	BindTexture(m_defaultTexture);
}

void Renderer::BeginFrame()
{
	ClearScreen(Rgba8(127, 127, 127, 255));
	m_rasterizerState.SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	m_depthStencilState.SetDepthStencilState(DepthTest::LESSEQUAL, true);
    m_blendState.SetBlendMode(BlendMode::ALPHA);
	m_samplerState.SetSamplerMode(SamplerMode::BILINEARWRAP);
	BindShader(nullptr);
	BindTexture(nullptr);
	SetTintColor(Rgba8::WHITE);

	DebugRenderBeginFrame();
}

void Renderer::EndFrame()
{
	DebugRenderEndFrame();

	// "Present" the back buffer by swapping the front (visible) and back (working) screen buffers
	m_mainScreen.swapChain->Present(0, 0);

	for (auto& screen : m_screens)
	{
		screen.swapChain->Present(0, 0);
	}
}

void Renderer::Shutdown()
{
	DebugRenderSystemShutdown();

	for (Texture* texture : m_loadedTextures)
	{
		// glDeleteTextures(1, &texture->m_openglTextureID);
		delete texture;
	}
	m_loadedTextures.clear();

	m_currentShader = nullptr;
	for (const Shader* shader : m_loadedShaders)
	{
		delete shader;
	}
	m_loadedShaders.clear();

	ReleaseRenderState();
	ReleaseRenderContext();
}

void Renderer::SetRenderTargets(int size, Texture* const* textures)
{
	constexpr int MAX_TARGETS = 16;

    for (auto& target : m_renderTargets)
        target = nullptr;

	if (!textures)
    {
        m_renderTargetSize = 1;
		m_renderTargets[0] = m_mainScreen.screenBuffer->m_rtv;
	}
    else
    {
        m_renderTargetSize = size;
        for (int i = 0; i < size && i < MAX_TARGETS; i++)
            m_renderTargets[i] = textures[i]->m_rtv;
	}


	m_deviceContext->OMSetRenderTargets(m_renderTargetSize, m_renderTargets, m_currentDSV);
}

const AABB2& Renderer::GetViewport()
{
	return m_viewport;
}

void Renderer::SetViewport(const AABB2& viewport)
{
	float width = (float)m_theConfig.m_window->GetClientDimensions().x;
	float height = (float)m_theConfig.m_window->GetClientDimensions().y;

	UINT paramNumViewports = 1;
	D3D11_VIEWPORT paramViewPort = {};
	paramViewPort.TopLeftX = viewport.m_mins.x * width;
	paramViewPort.TopLeftY = viewport.m_mins.y * height;
	paramViewPort.Width = viewport.GetDimensions().x * width;
	paramViewPort.Height = viewport.GetDimensions().y * height;
	paramViewPort.MinDepth = 0.0f;
	paramViewPort.MaxDepth = 1.0f;

	m_deviceContext->RSSetViewports(paramNumViewports, &paramViewPort);
	m_viewport = AABB2(paramViewPort.TopLeftX, paramViewPort.TopLeftY, paramViewPort.TopLeftX + paramViewPort.Width, paramViewPort.TopLeftY + paramViewPort.Height);
}

void Renderer::ReleaseRenderState()
{
	// delete default shader
	delete m_defaultShader;
	m_defaultShader = nullptr;
	m_defaultVF_PCU = VertexFormat();
	m_defaultVF_PNCU = VertexFormat();

	// delete default texture
	delete m_defaultTexture;
	m_defaultTexture = nullptr;

	// delete default vbo
	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	// delete shader cbo
	delete m_modelCBO;
	m_modelCBO = nullptr;
	delete m_cameraCBO;
	m_cameraCBO = nullptr;
	delete m_lightCBO;
	m_lightCBO = nullptr;

	for (auto& cbo : m_customCBOs)
	{
		delete cbo;
		cbo = nullptr;
	}

	m_blendState.Release();
	m_samplerState.Release();
	m_rasterizerState.Release();
	m_depthStencilState.Release();

	delete m_depthStencilState.m_depthStencilTexture;
	m_depthStencilState.m_depthStencilTexture = nullptr;
}

void Renderer::SetRenderStates()
{
	if (m_rasterizerState.m_stateChanged)
	{
		DX_SAFE_RELEASE(m_rasterizerState.m_d3dRasterizerState);

		HRESULT result = {};

		D3D11_RASTERIZER_DESC paramRasterDesc = {};
		paramRasterDesc.FillMode = (D3D11_FILL_MODE)GetD3DConstant(m_rasterizerState.m_fillMode);
		paramRasterDesc.CullMode = (D3D11_CULL_MODE)GetD3DConstant(m_rasterizerState.m_cullMode);
		paramRasterDesc.FrontCounterClockwise = GetD3DConstant(m_rasterizerState.m_windingOrder);
		paramRasterDesc.DepthClipEnable = true;
		paramRasterDesc.AntialiasedLineEnable = true;

		result = m_device->CreateRasterizerState(&paramRasterDesc, &m_rasterizerState.m_d3dRasterizerState);
		ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateRasterizerState");
		m_deviceContext->RSSetState(m_rasterizerState.m_d3dRasterizerState);

		m_rasterizerState.m_stateChanged = false;
	}

	if (m_depthStencilState.m_stateChanged)
	{
		DX_SAFE_RELEASE(m_depthStencilState.m_depthStencilState);

		HRESULT result = {};

		D3D11_DEPTH_STENCIL_DESC paramDepthDesc = {};

		paramDepthDesc.DepthEnable = true;
		paramDepthDesc.DepthWriteMask = m_depthStencilState.m_depthMask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        paramDepthDesc.DepthFunc = (D3D11_COMPARISON_FUNC)GetD3DConstant(m_depthStencilState.m_depthTest);
		paramDepthDesc.StencilEnable = FALSE;
		paramDepthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		paramDepthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
        const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
        { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
		paramDepthDesc.FrontFace = defaultStencilOp;
		paramDepthDesc.BackFace = defaultStencilOp;

		if (m_depthStencilState.m_stencilTest && m_depthStencilState.m_stencilWrite)
        {
            paramDepthDesc.StencilEnable = true;
            paramDepthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;
            paramDepthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
            paramDepthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
		}
		else if (m_depthStencilState.m_stencilTest)
        {
            paramDepthDesc.StencilEnable = true;
            paramDepthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;
			paramDepthDesc.StencilWriteMask = 0;
		}
		else if (m_depthStencilState.m_stencilWrite)
        {
            paramDepthDesc.StencilEnable = true;
            paramDepthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            paramDepthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
            paramDepthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
            paramDepthDesc.StencilReadMask = 0;
		}

		result = m_device->CreateDepthStencilState(&paramDepthDesc, &m_depthStencilState.m_depthStencilState);
		ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateRasterizerState");
		m_deviceContext->OMSetDepthStencilState(m_depthStencilState.m_depthStencilState, 0);

		m_depthStencilState.m_stateChanged = false;
	}

	if (m_blendState.m_stateChanged)
	{
		float blendFactor[4] = {};
		m_deviceContext->OMSetBlendState(m_blendState.m_blendStates[(int)m_blendState.m_blendMode], blendFactor, 0xffffffff);

		m_blendState.m_stateChanged = false;
	}

	if (m_samplerState.m_stateChanged)
	{
		m_deviceContext->PSSetSamplers(0, 1, &m_samplerState.m_samplerStates[(int)m_samplerState.m_samplerMode]);

		m_samplerState.m_stateChanged = false;
	}
}

void Renderer::ReleaseRenderContext()
{
	delete m_mainScreen.screenBuffer;
	m_mainScreen.screenBuffer = nullptr;
	DX_SAFE_RELEASE(m_mainScreen.swapChain);

	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);

#ifdef ENGINE_DEBUG_RENDER
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;
	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}

void Renderer::ClearScreen(const Rgba8& color)
{
	ClearScreen(color, m_mainScreen.screenBuffer);
	ClearDepth();
}

void Renderer::ClearScreen(const Rgba8& color, Texture* renderTarget)
{
	static float floatColor[4] = {};
	color.GetAsFloats(floatColor);

	m_deviceContext->ClearRenderTargetView(renderTarget->m_rtv, floatColor);
}

void Renderer::ClearDepth(float value /*= 1.0f*/)
{
	if (m_currentDSV)
		m_deviceContext->ClearDepthStencilView(m_currentDSV, D3D11_CLEAR_DEPTH, value, 0);
}

void Renderer::ClearStencil(unsigned char value /*= 1.0f*/)
{
    if (m_currentDSV)
        m_deviceContext->ClearDepthStencilView(m_currentDSV, D3D11_CLEAR_STENCIL, 1.0f, value);
}

ModelConstants& Renderer::GetModelConstants()
{
	return m_modelConstants;
}

LightConstants& Renderer::GetLightConstants()
{
	return m_lightConstants;
}

void Renderer::UpdateLights()
{
	SetLightState();
}

void Renderer::SetTintColor(const Rgba8& color)
{
	m_modelConstants.TintColor.x = NormalizeByte(color.r);
	m_modelConstants.TintColor.y = NormalizeByte(color.g);
	m_modelConstants.TintColor.z = NormalizeByte(color.b);
	m_modelConstants.TintColor.w = NormalizeByte(color.a);
}

void Renderer::SetModelMatrix(const Mat4x4& modelMatrix)
{
	m_modelConstants.ModelMatrix = modelMatrix;
}

void Renderer::SetModelState()
{
	CopyCPUToGPU(&m_modelConstants, sizeof(ModelConstants), m_modelCBO);
	BindConstantBuffer(MODEL_CONSTANT_BUFFER_SLOT, m_modelCBO);
}

void Renderer::SetLightState()
{
	CopyCPUToGPU(&m_lightConstants, sizeof(LightConstants), m_lightCBO);
	BindConstantBuffer(LIGHT_CONSTANT_BUFFER_SLOT, m_lightCBO);
}

void Renderer::BeginCamera(const Camera& camera, const Window* window)
{
	if (window == nullptr)
    {
        m_deviceContext->OMSetRenderTargets(1, &m_mainScreen.screenBuffer->m_rtv, m_currentDSV);
	}
	else
	{
		m_deviceContext->OMSetRenderTargets(1, &GetScreenTexture(window)->m_rtv, m_currentDSV);
	}
	
	CameraConstants& constant = m_cameraConstants;
	constant = {};
	constant.ProjectionMatrix = camera.GetProjectionMatrix();
	constant.ProjectionMatrix.Append(camera.GetRenderMatrix().GetOrthonormalInverse());
	constant.ViewMatrix = camera.GetViewMatrix();

	CopyCPUToGPU(&constant, sizeof(CameraConstants), m_cameraCBO);
	BindConstantBuffer(CAMERA_CONSTANT_BUFFER_SLOT, m_cameraCBO);

	m_modelConstants.ModelMatrix.SetIdentity();
	m_modelConstants.TintColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

	SetViewport(camera.GetViewport());
}

void Renderer::EndCamera(const Camera& camera)
{
	UNUSED(camera);
	SetViewport(AABB2::ZERO_TO_ONE);
}

void Renderer::DrawVertexArray(int numVertex, const Vertex_PCU* vertexArray)
{
	if (numVertex == 0)
		return;

	CopyCPUToGPU(vertexArray, m_immediateVBO->GetStride() * numVertex, m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertex);
}

void Renderer::DrawVertexArray(VertexList vertices)
{
	DrawVertexArray((int) vertices.size(), vertices.data());
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0*/)
{
    if (vertexCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(vbo);

#ifdef ALWAYS_USE_DRAW_INDEXED
	IndexBuffer* ibo = CreateIndexBuffer(sizeof(unsigned int) * vertexCount);
	std::vector<unsigned int> indices;
	indices.reserve(vertexCount);
	for (int i = 0; i < vertexCount; i++)
	{
		indices.push_back(i);
	}
	CopyCPUToGPU(indices.data(), sizeof(unsigned int) * vertexCount, ibo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(vertexCount, 0, vertexOffset);
	delete ibo;
#else
	m_deviceContext->Draw(vertexCount, vertexOffset);
#endif
}

void Renderer::DrawVertexBuffer(int slots, VertexBuffer** vbo, int vertexCount, int vertexOffset /*= 0*/)
{
    if (vertexCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(slots, vbo);

#ifdef ALWAYS_USE_DRAW_INDEXED
	IndexBuffer* ibo = CreateIndexBuffer(sizeof(unsigned int) * vertexCount);
	std::vector<unsigned int> indices;
	indices.reserve(vertexCount);
	for (int i = 0; i < vertexCount; i++)
	{
		indices.push_back(i);
	}
	CopyCPUToGPU(indices.data(), sizeof(unsigned int) * vertexCount, ibo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(vertexCount, 0, vertexOffset);
	delete ibo;
#else
	m_deviceContext->Draw(vertexCount, vertexOffset);
#endif
}

void Renderer::DrawIndexedVertexBuffer(IndexBuffer* ibo, VertexBuffer* vbo, int indexCount, int indexOffset, int vertexOffset)
{
	if (indexCount == 0)
		return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void Renderer::DrawIndexedVertexBuffer(IndexBuffer* ibo, int slots, VertexBuffer** vbo, int indexCount, int indexOffset /*= 0*/, int vertexOffset /*= 0*/)
{
    if (indexCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(slots, vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void Renderer::DrawInstancedVertexBuffer(VertexBuffer** vbo, int vertexCount, int instanceCount, int vertexOffset /*= 0*/, int instanceOffset /*= 0*/)
{
    if (vertexCount == 0 || instanceCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(vbo[0], vbo[1]);

#ifdef ALWAYS_USE_DRAW_INDEXED
	IndexBuffer* ibo = CreateIndexBuffer(sizeof(unsigned int) * vertexCount);
	std::vector<unsigned int> indices;
	indices.reserve(vertexCount);
	for (int i = 0; i < vertexCount; i++)
	{
		indices.push_back(i);
	}
	CopyCPUToGPU(indices.data(), sizeof(unsigned int) * vertexCount, ibo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawInstancedIndexed(vertexCount, instanceCount, 0, vertexOffset, 0);
	delete ibo;
#else
	m_deviceContext->DrawInstanced(vertexCount, instanceCount, vertexOffset, instanceOffset);
#endif
}

void Renderer::DrawInstancedVertexBuffer(int slots, VertexBuffer** vbo, int vertexCount, int instanceCount, int vertexOffset /*= 0*/, int instanceOffset /*= 0*/)
{
    if (vertexCount == 0 || instanceCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(slots, vbo);

#ifdef ALWAYS_USE_DRAW_INDEXED
	IndexBuffer* ibo = CreateIndexBuffer(sizeof(unsigned int) * vertexCount);
	std::vector<unsigned int> indices;
	indices.reserve(vertexCount);
	for (int i = 0; i < vertexCount; i++)
	{
		indices.push_back(i);
	}
	CopyCPUToGPU(indices.data(), sizeof(unsigned int) * vertexCount, ibo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawInstancedIndexed(vertexCount, instanceCount, 0, vertexOffset, 0);
	delete ibo;
#else
	m_deviceContext->DrawInstanced(vertexCount, instanceCount, vertexOffset, instanceOffset);
#endif
}

void Renderer::DrawIndexedInstancedVertexBuffer(IndexBuffer* ibo, VertexBuffer** vbo, int indexCount, int instanceCount, int indexOffset /*= 0*/, int vertexOffset /*= 0*/, int instanceOffset /*= 0*/)
{
    if (indexCount == 0 || instanceCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(vbo[0], vbo[1]);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexedInstanced(indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}

void Renderer::DrawIndexedInstancedVertexBuffer(int slots, IndexBuffer* ibo, VertexBuffer** vbo, int indexCount, int instanceCount, int indexOffset /*= 0*/, int vertexOffset /*= 0*/, int instanceOffset /*= 0*/)
{
    if (indexCount == 0 || instanceCount == 0)
        return;

	SetRenderStates();
	SetModelState();
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	BindVertexBuffer(slots, vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexedInstanced(indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}

void Renderer::DrawPostprocessRect(const Window* window)
{
	if (window == nullptr)
    {
        m_deviceContext->OMSetRenderTargets(1, &m_mainScreen.screenBuffer->m_rtv, m_depthStencilState.m_depthStencilView);
	}
	else
    {
        m_deviceContext->OMSetRenderTargets(1, &GetScreenTexture(window)->m_rtv, m_depthStencilState.m_depthStencilView);
	}
    Camera ortho;
    ortho.SetOrthoView(AABB2::ZERO_TO_ONE.m_mins, AABB2::ZERO_TO_ONE.m_maxs);
    ortho.SetViewport(AABB2::ZERO_TO_ONE);

    CameraConstants constant = {};
    constant.ProjectionMatrix = ortho.GetProjectionMatrix();
    constant.ProjectionMatrix.Append(ortho.GetRenderMatrix().GetOrthonormalInverse());
    constant.ViewMatrix = ortho.GetViewMatrix();

    CopyCPUToGPU(&constant, sizeof(CameraConstants), m_cameraCBO);

    ModelConstants modelConsts = m_modelConstants;

    m_modelConstants.ModelMatrix.SetIdentity();
    m_modelConstants.TintColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

    Vertex_PCU verts[6] = {};
    verts[0].m_position = Vec3(0, 1, 0);
    verts[0].m_uvTexCoords = Vec2(0, 0);
    verts[1].m_position = Vec3(0, 0, 0);
    verts[1].m_uvTexCoords = Vec2(0, 1);
    verts[2].m_position = Vec3(1, 1, 0);
    verts[2].m_uvTexCoords = Vec2(1, 0);
    verts[3].m_position = Vec3(1, 0, 0);
    verts[3].m_uvTexCoords = Vec2(1, 1);
    verts[4].m_position = Vec3(1, 1, 0);
    verts[4].m_uvTexCoords = Vec2(1, 0);
    verts[5].m_position = Vec3(0, 0, 0);
    verts[5].m_uvTexCoords = Vec2(0, 1);

    DrawVertexArray(6, verts);

    m_modelConstants = modelConsts;
    CopyCPUToGPU(&m_cameraConstants, sizeof(CameraConstants), m_cameraCBO);
    m_deviceContext->OMSetRenderTargets(m_renderTargetSize, m_renderTargets, m_currentDSV);
}

void Renderer::DrawNDCFullscreenRect()
{
    Vertex_PCU verts[6] = {};
    verts[0].m_position = Vec3(-1, 1, 0);
    verts[0].m_uvTexCoords = Vec2(0, 0);
    verts[1].m_position = Vec3(-1, -1, 0);
    verts[1].m_uvTexCoords = Vec2(0, 1);
    verts[2].m_position = Vec3(1, 1, 0);
    verts[2].m_uvTexCoords = Vec2(1, 0);
    verts[3].m_position = Vec3(1, -1, 0);
    verts[3].m_uvTexCoords = Vec2(1, 1);
    verts[4].m_position = Vec3(1, 1, 0);
    verts[4].m_uvTexCoords = Vec2(1, 0);
    verts[5].m_position = Vec3(-1, -1, 0);
    verts[5].m_uvTexCoords = Vec2(0, 1);

    DrawVertexArray(6, verts);
}

void Renderer::SetCullMode(CullMode cullMode)
{
	m_rasterizerState.SetCullMode(cullMode);
}

void Renderer::SetFillMode(FillMode fillMode)
{
	m_rasterizerState.SetFillMode(fillMode);
}

void Renderer::SetWindingOrder(WindingOrder windingOrder)
{
	m_rasterizerState.SetWindingOrder(windingOrder);
}

void Renderer::SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder)
{
	m_rasterizerState.SetRasterizerState(cullMode, fillMode, windingOrder);
}

void Renderer::SetDepthTest(DepthTest depthTest)
{
	m_depthStencilState.SetDepthTest(depthTest);
}

void Renderer::SetDepthMask(bool writeDepth)
{
	m_depthStencilState.SetDepthMask(writeDepth);
}

void Renderer::SetDepthStencilState(DepthTest depthTest, bool writeDepth)
{
	m_depthStencilState.SetDepthStencilState(depthTest, writeDepth);
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_blendState.SetBlendMode(blendMode);
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_samplerState.SetSamplerMode(samplerMode);
}

DepthStencilState& Renderer::GetDepthStencilState()
{
	return m_depthStencilState;
}

void Renderer::InitializeCustomConstantBuffer(int slot, size_t size)
{
	ConstantBuffer*& cbo = m_customCBOs[slot - CUSTOM_CONSTANT_BUFFER_SLOT_START];
	if (cbo)
		ERROR_AND_DIE("Reinitialization of custom cbo");
	cbo = CreateConstantBuffer(size);
}

Texture* Renderer::GetTextureForFileName(const char* imageFilePath)
{
	for (Texture* texture : m_loadedTextures)
	{
		if (texture->GetImageFilePath() == imageFilePath) return texture;
	}
	return nullptr;
}

BitmapFont* Renderer::GetBitmapFontForFileName(const char* fontFilePathNameWithNoExtension)
{
	for (BitmapFont* font : m_loadedBitmapFonts)
	{
		if (font->m_fontFilePathNameWithNoExtension == fontFilePathNameWithNoExtension) return font;
	}
	return nullptr;
}

//-----------------------------------------------------------------------------------------------
// Sample code for loading an image from disk and creating an OpenGL texture from its data.
// 
// Game code calls RenderContext::CreateOrGetTextureFromFile(), which in turn will
//	check that name amongst the registry of already-loaded textures (by name).  If that image
//	has already been loaded, the renderer simply returns the Texture* it already has.  If the
//	image has not been loaded before, CreateTextureFromFile() gets called internally, which in
//	turn calls CreateTextureFromData.  The new Texture* is then added to the registry of
//	already-loaded textures, and then returned.
//------------------------------------------------------------------------------------------------


void Renderer::SetCustomConstantBuffer(int slot, const void* data)
{
	ConstantBuffer* cbo = m_customCBOs[slot - CUSTOM_CONSTANT_BUFFER_SLOT_START];
	if (!cbo)
		return;

	CopyCPUToGPU(data, cbo->m_size, cbo);
	BindConstantBuffer(slot, cbo);
}

int Renderer::AddScreen(Window* window)
{
	HRESULT result;

	m_screens.emplace_back();
	ScreenTarget& screen = m_screens.back();
	screen.window = window;

    DXGI_SWAP_CHAIN_DESC paramSwapChainDesc = {};
	paramSwapChainDesc.BufferDesc.Width = window->GetClientDimensions().x;
	paramSwapChainDesc.BufferDesc.Height = window->GetClientDimensions().y;
	paramSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	paramSwapChainDesc.SampleDesc.Count = 1;
	paramSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	paramSwapChainDesc.BufferCount = 2;
	paramSwapChainDesc.OutputWindow = HWND(window->GetHwnd());
	paramSwapChainDesc.Windowed = true;
	paramSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	screen.screenBuffer = new Texture();
	screen.screenBuffer->m_info.name = "SCREEN_BUFFER";
	screen.screenBuffer->m_info.dimensions = window->GetClientDimensions();

	IDXGIDevice* dxgiDevice;
	result = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to cast ID3D11Device -> IDXGIDevice");

	IDXGIAdapter* dxgiAdapter;
	result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to cast ID3D11Device -> IDXGIAdapter");

	IDXGIFactory* dxgiFactory;
	result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to cast ID3D11Device -> IDXGIFactory");

	result = dxgiFactory->CreateSwapChain(m_device, &paramSwapChainDesc, &screen.swapChain);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call IDXGIFactory::CreateSwapChain");

	DX_SAFE_RELEASE(dxgiFactory);
	DX_SAFE_RELEASE(dxgiAdapter);
	DX_SAFE_RELEASE(dxgiDevice);

	screen.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&screen.screenBuffer->m_texture);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call IDXGISwapChain::GetBuffer");
	m_device->CreateRenderTargetView(screen.screenBuffer->m_texture, nullptr, &screen.screenBuffer->m_rtv);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateRenderTargetView");

	return int(m_screens.size()) - 1;
}

void Renderer::RemoveScreen(Window* window)
{
	for (auto ite = m_screens.begin(); ite != m_screens.end(); ++ite)
	{
	    if (ite->window == window)
	    {
			ScreenTarget screen = *ite;
			m_screens.erase(ite);

			delete screen.screenBuffer;
			DX_SAFE_RELEASE(screen.swapChain);

			return;
	    }
	}
}

Texture* Renderer::GetScreenTexture(const Window* window) const
{
	if (window == nullptr || window == m_mainScreen.window)
		return m_mainScreen.screenBuffer;

	for (auto& screen : m_screens)
		if (window == screen.window)
			return screen.screenBuffer;

	return nullptr;
}

void Renderer::SetDepthTarget(Texture* texture)
{
	if (m_currentDSV == m_depthStencilState.m_depthStencilView)
	{
		if (texture)
		{
			// default -> custom
			m_depthBuffer = texture;

			D3D11_DEPTH_STENCIL_VIEW_DESC pDepthStencilDesc = {};
			pDepthStencilDesc.Flags = 0;
			pDepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			pDepthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			pDepthStencilDesc.Texture2D.MipSlice = 0;

			HRESULT result = m_device->CreateDepthStencilView(m_depthBuffer->m_texture, &pDepthStencilDesc, &m_currentDSV);
			ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateDepthStencilView");
		}
	}
	else
	{
		if (!texture)
		{
			// custom -> default
			DX_SAFE_RELEASE(m_currentDSV);

			m_depthBuffer = nullptr;
			m_currentDSV = m_depthStencilState.m_depthStencilView;
		}
		else if (texture != m_depthBuffer)
		{
			// custom -> custom
			DX_SAFE_RELEASE(m_currentDSV);

			m_depthBuffer = texture;

			D3D11_DEPTH_STENCIL_VIEW_DESC pDepthStencilDesc = {};
			pDepthStencilDesc.Flags = 0;
			pDepthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			pDepthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			pDepthStencilDesc.Texture2D.MipSlice = 0;

			HRESULT result = m_device->CreateDepthStencilView(m_depthBuffer->m_texture, &pDepthStencilDesc, &m_currentDSV);
			ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateDepthStencilView");
		}
	}
}

Texture* Renderer::CreateTexture(IntVec2 dimensions, bool isR32, bool isDepth, bool isTarget)
{
	static std::atomic_int textuteCounter = 0;

	if (dimensions.x == -1)
		dimensions.x = m_theConfig.m_window->GetClientDimensions().x;
	if (dimensions.y == -1)
		dimensions.y = m_theConfig.m_window->GetClientDimensions().y;

	HRESULT result = {};

	D3D11_TEXTURE2D_DESC pTextureDesc = {};
	pTextureDesc.Width = dimensions.x;
	pTextureDesc.Height = dimensions.y;
	pTextureDesc.MipLevels = 1;
	pTextureDesc.ArraySize = 1;
	pTextureDesc.Format = isR32 ? DXGI_FORMAT_R32_FLOAT : isDepth ? DXGI_FORMAT_R24G8_TYPELESS : DXGI_FORMAT_R8G8B8A8_UNORM;
	pTextureDesc.SampleDesc.Count = 1;
	pTextureDesc.Usage = (isTarget || isDepth) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	pTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (isTarget)
		pTextureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if (isDepth)
		pTextureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

	std::vector<Rgba8> bufferData;
	bufferData.resize((size_t)dimensions.x* (size_t)dimensions.y);

	D3D11_SUBRESOURCE_DATA pSubResData = {};
	pSubResData.pSysMem = bufferData.data();
	pSubResData.SysMemPitch = sizeof(Rgba8) * dimensions.x;

	Texture* texture = new Texture();
	texture->m_info.name = (isDepth ? "DEPTH_" : "TEXTURE_") + std::to_string(textuteCounter++);
	texture->m_info.dimensions = dimensions;

	result = m_device->CreateTexture2D(&pTextureDesc, &pSubResData, &texture->m_texture);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateTexture2D");

	D3D11_SHADER_RESOURCE_VIEW_DESC pShaderResourceViewDesc = {};
	pShaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	pShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	pShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	pShaderResourceViewDesc.Texture2D.MipLevels = (UINT)-1;

	result = m_device->CreateShaderResourceView(texture->m_texture, isDepth ? &pShaderResourceViewDesc : nullptr, &texture->m_srv);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateShaderResourceView");
	SetDebugName(texture->m_texture, texture->m_info.name.c_str());

	if (isTarget)
	{
		result = m_device->CreateRenderTargetView(texture->m_texture, nullptr, &texture->m_rtv);
		ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateRenderTargetView");
	}

	m_loadedTextures.push_back(texture);
	return texture;
}

Texture* Renderer::CreateTexture(const TextureCreateInfo& info)
{
    HRESULT result = {};

    D3D11_TEXTURE2D_DESC pTextureDesc = {};
    pTextureDesc.Width = info.dimensions.x;
    pTextureDesc.Height = info.dimensions.y;
    pTextureDesc.MipLevels = 1;
    pTextureDesc.ArraySize = 1;
    pTextureDesc.Format = (DXGI_FORMAT) GetD3DConstant(info.format);
    pTextureDesc.SampleDesc.Count = 1;
    pTextureDesc.Usage = (D3D11_USAGE) GetD3DConstant(info.memoryHint);
    pTextureDesc.BindFlags = GetD3DFlags(info.bindFlags);
	pTextureDesc.CPUAccessFlags = info.memoryHint == EMemoryHint::DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;

    D3D11_SUBRESOURCE_DATA pSubResData = {};
    pSubResData.pSysMem = info.initData;
    pSubResData.SysMemPitch = static_cast<UINT>(info.initDataLength / info.dimensions.y);

    Texture* texture = new Texture();
    texture->m_info.name = info.name;
    texture->m_info.dimensions = info.dimensions;

    result = m_device->CreateTexture2D(&pTextureDesc, info.memoryHint == EMemoryHint::DYNAMIC ? nullptr : &pSubResData, &texture->m_texture);
    ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateTexture2D");
    result = m_device->CreateShaderResourceView(texture->m_texture, nullptr, &texture->m_srv);
    ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateShaderResourceView");
    SetDebugName(texture->m_texture, info.name.c_str());

    return texture;
}

void Renderer::DeleteTexture(Texture*& texture)
{
	if (!texture)
		return;

	for (auto ite = m_loadedTextures.begin(); ite != m_loadedTextures.end(); ite++)
	{
		if (*ite == texture)
		{
			m_loadedTextures.erase(ite);
			delete texture;
			texture = nullptr;
			return;
		}
	}

	ERROR_AND_DIE("Texture not handled by renderer");
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromFile(const char* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}


Texture* Renderer::GetDepthStencilTexture()
{
	return m_depthStencilState.m_depthStencilTexture;
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile(const char* imageFilePath)
{
	Image image(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(image);

	m_loadedTextures.push_back(newTexture);
	return newTexture;
}


Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	TextureCreateInfo info;
	info.name = image.GetImageFilePath();
	info.dimensions = image.GetDimensions();
	info.initData = image.GetRawData();
	info.initDataLength = image.GetRawDataSize();

	return CreateTexture(info);
}

//------------------------------------------------------------------------------------------------
// Texture* Renderer::CreateTextureFromData(const char* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
// {
// 	// Check if the load was successful
// 	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
// 	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
// 	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));
// 
// 	Texture* newTexture = new Texture();
// 	newTexture->m_name = name;
// 	newTexture->m_dimensions = dimensions;

// 	// Enable OpenGL texturing
// 	glEnable(GL_TEXTURE_2D);
// 
// 	// Tell OpenGL that our pixel data is single-byte aligned
// 	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
// 
// 	// Ask OpenGL for an unused texName (ID number) to use for this texture
// 	glGenTextures(1, (GLuint*)&newTexture->m_openglTextureID);
// 
// 	// Tell OpenGL to bind (set) this as the currently active texture
// 	glBindTexture(GL_TEXTURE_2D, newTexture->m_openglTextureID);
// 
// 	// Set texture clamp vs. wrap (repeat) default settings
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP or GL_REPEAT
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP or GL_REPEAT
// 
// 																	// Set magnification (texel > pixel) and minification (texel < pixel) filters
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
// 
// 																		 // Pick the appropriate OpenGL format (RGB or RGBA) for this texel data
// 	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
// 	if (bytesPerTexel == 3)
// 	{
// 		bufferFormat = GL_RGB;
// 	}
// 	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; technically allows us to translate into a different texture format as we upload to OpenGL
// 
// 										  // Upload the image texel data (raw pixels bytes) to OpenGL under this textureID
// 	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
// 		GL_TEXTURE_2D,		// Creating this as a 2d texture
// 		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
// 		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
// 		dimensions.x,		// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
// 		dimensions.y,		// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
// 		0,					// Border size, in texels (must be 0 or 1, recommend 0)
// 		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
// 		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
// 		texelData);		// Address of the actual pixel data bytes/buffer in system memory
// 
// 	return newTexture;
// }


//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture(const Texture* texture, int index)
{
	ID3D11ShaderResourceView* pSRV;
	if (texture)
	{
		pSRV = texture->m_srv;
		m_currentTexture[index] = (Texture*)texture;
	}
	else
	{
		if (index == 0)
		{
			// guarantee texture at slot 0 is always not null
			pSRV = m_defaultTexture->m_srv;
			m_currentTexture[index] = m_defaultTexture;
		}
		else
		{
			pSRV = nullptr;
			m_currentTexture[index] = nullptr;
		}
	}

	// set shader resource
	m_deviceContext->PSSetShaderResources(index, 1, &pSRV);
}


const VertexFormat& Renderer::GetDefaultVF_PCU()
{
	return m_defaultVF_PCU;
}

const VertexFormat& Renderer::GetDefaultVF_PNCU()
{
	return m_defaultVF_PNCU;
}

Shader* Renderer::CreateOrGetShader(const char* shaderName)
{
	std::string shaderStr = shaderName;
	if (shaderStr.find("Data/Shaders/") != std::string::npos)
	{
		shaderStr = shaderStr.substr(13);
		shaderName = shaderStr.c_str();
	}

	Shader* shader = GetShaderForName(shaderName);
	if (shader)
		return shader;

	return CreateShaderFromFile(shaderName);
}

Shader* Renderer::CreateShaderFromFile(const char* shaderName)
{
	std::string fileName = std::string("Data/Shaders/") + shaderName + std::string(".hlsl");
	std::string source;
	if (FileReadToString(source, fileName) < 0)
		ERROR_AND_DIE(Stringf("Shader source file not found: ", fileName.c_str()));

	std::string strShaderName = shaderName;
	bool isLit = strShaderName.find("Lit") != std::string::npos;
	return CreateShader(shaderName, source, isLit ? m_defaultVF_PNCU : m_defaultVF_PCU);
}

void Renderer::BindShaderByName(const char* shaderName)
{
	Shader* shader = nullptr;
	if (shaderName)
	{
		shader = CreateOrGetShader(shaderName);
	}
	BindShader(shader);
}

void Renderer::BindShader(Shader* shader)
{
	if (!shader)
		shader = m_defaultShader;

	m_currentShader = shader;

	m_deviceContext->IASetInputLayout(m_currentShader->m_inputLayout);
	m_deviceContext->VSSetShader(m_currentShader->m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader(m_currentShader->m_pixelShader, nullptr, 0);
}

Shader* Renderer::GetShaderForName(const char* shaderName)
{
	std::string shaderStr = shaderName;
	if (shaderStr.find("Data/Shaders/") != std::string::npos)
	{
		shaderStr = shaderStr.substr(13);
		shaderName = shaderStr.c_str();
	}

	if (m_defaultShader->GetName() == shaderName)
	{
		return m_defaultShader;
	}
	for (auto& shader : m_loadedShaders)
	{
		if (shader->GetName() == shaderName) 
			return shader;
	}
	return nullptr;
}

Shader* Renderer::CreateShader(const char* shaderName, std::string& shaderSource, const VertexFormat& vertexFormat)
{
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	shaderConfig.m_formats.push_back(vertexFormat);

	Shader* shader = new Shader(shaderConfig);

	HRESULT result = {};

	std::vector<uint8_t> dxVSBytecode;
	std::vector<uint8_t> dxPSBytecode;

	CompileShaderToByteCode(dxVSBytecode, shaderName, shaderSource, shader->m_config.m_vertexEntryPoint.c_str(), "vs_5_0");
	CompileShaderToByteCode(dxPSBytecode, shaderName, shaderSource, shader->m_config.m_pixelEntryPoint.c_str(), "ps_5_0");

	// create vs
	result = m_device->CreateVertexShader(dxVSBytecode.data(), dxVSBytecode.size(), nullptr, &shader->m_vertexShader);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateVertexShader");

	// create layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
	vertexFormat.GetInputElements(layoutDesc);
	result = m_device->CreateInputLayout(layoutDesc.data(), (UINT)layoutDesc.size(), dxVSBytecode.data(), dxVSBytecode.size(), &shader->m_inputLayout);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateInputLayout");

	// create ps
	result = m_device->CreatePixelShader(dxPSBytecode.data(), dxPSBytecode.size(), nullptr, &shader->m_pixelShader);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreatePixelShader");

	m_loadedShaders.push_back(shader);
	return shader;
}

Shader* Renderer::CreateShader(const char* shaderName, std::string& shaderSource, const std::vector<VertexFormat>& vertexFormats)
{
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	shaderConfig.m_formats = vertexFormats;

	Shader* shader = new Shader(shaderConfig);

	HRESULT result = {};

	std::vector<uint8_t> dxVSBytecode;
	std::vector<uint8_t> dxPSBytecode;

	CompileShaderToByteCode(dxVSBytecode, shaderName, shaderSource, shader->m_config.m_vertexEntryPoint.c_str(), "vs_5_0");
	CompileShaderToByteCode(dxPSBytecode, shaderName, shaderSource, shader->m_config.m_pixelEntryPoint.c_str(), "ps_5_0");

	// create vs
	result = m_device->CreateVertexShader(dxVSBytecode.data(), dxVSBytecode.size(), nullptr, &shader->m_vertexShader);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateVertexShader");

	// create layout
	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
	for (auto& vertexFormat : vertexFormats)
		vertexFormat.GetInputElements(layoutDesc);
	result = m_device->CreateInputLayout(layoutDesc.data(), (UINT)layoutDesc.size(), dxVSBytecode.data(), dxVSBytecode.size(), &shader->m_inputLayout);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateInputLayout");

	// create ps
	result = m_device->CreatePixelShader(dxPSBytecode.data(), dxPSBytecode.size(), nullptr, &shader->m_pixelShader);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreatePixelShader");

	m_loadedShaders.push_back(shader);
	return shader;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, const char* name, std::string& source, const char* entryPoint, const char* target)
{
	HRESULT result = {};

	UINT paramShaderFlags = 0;
#ifdef ENGINE_DEBUG_RENDER
	paramShaderFlags |= D3DCOMPILE_DEBUG;
	paramShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	paramShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob* pCode = nullptr;
	ID3DBlob* pErrMsg = nullptr;

	result = ::D3DCompile(source.c_str(), source.length(), name, nullptr, nullptr, entryPoint, target, paramShaderFlags, 0, &pCode, &pErrMsg);
	ASSERT_OR_DIE(SUCCEEDED(result), Stringf("Failed to call ::D3DCompile: \n\nSHADER CODE:\n%s\n\nERROR MESSAGE:\n%s", source.c_str(), pErrMsg->GetBufferPointer()));
	outByteCode.resize(pCode->GetBufferSize());
	memcpy(outByteCode.data(), pCode->GetBufferPointer(), (size_t)pCode->GetBufferSize());
	DX_SAFE_RELEASE(pCode);
	DX_SAFE_RELEASE(pErrMsg);

	return true;
}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size, const int stride /*= sizeof(Vertex_PCU)*/)
{
	VertexBuffer* buffer = new VertexBuffer(size);
	buffer->m_stride = stride;

	HRESULT result = {};

	D3D11_BUFFER_DESC paramBufferDesc = {};
	paramBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paramBufferDesc.ByteWidth = (UINT)size;
	paramBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	paramBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	result = m_device->CreateBuffer(&paramBufferDesc, nullptr, &buffer->m_buffer);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateBuffer");

	return buffer;
}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size, const VertexFormat* format)
{
	return CreateVertexBuffer(size, format->GetVertexStride());
}

VertexBuffer* Renderer::CreateInstanceBuffer(const size_t size, const VertexFormat* format)
{
	return CreateVertexBuffer(size, format->GetVertexStride());
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo)
{
	if (size == 0)
	{
		return;
	}

	HRESULT result = {};

	if (vbo->m_size < size)
	{
		DX_SAFE_RELEASE(vbo->m_buffer);
		//size_t newBufferSize = vbo->m_size + (vbo->m_size >> 1); // grow by half
		VertexBuffer* newvbo = CreateVertexBuffer(size); // size > newBufferSize ? size : newBufferSize);
		vbo->m_buffer = newvbo->m_buffer;
		vbo->m_size = newvbo->m_size;
		newvbo->m_buffer = nullptr;
		delete newvbo;
	}

	D3D11_MAPPED_SUBRESOURCE paramBufferSource = {};
	result = m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &paramBufferSource);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11DeviceContext::Map");
	memcpy(paramBufferSource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

void Renderer::BindVertexBuffer(const VertexBuffer* vbo)
{
	UINT pStrides = vbo->GetStride();
	UINT pOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &pStrides, &pOffset);
}

void Renderer::BindVertexBuffer(const VertexBuffer* vertexBuffer, const VertexBuffer* instanceBuffer)
{
	ID3D11Buffer* pBuffers[2] = { vertexBuffer->m_buffer, instanceBuffer->m_buffer };
	UINT          pStrides[2] = { vertexBuffer->GetStride(), instanceBuffer->GetStride() };
	UINT          pOffsets[2] = {};
	m_deviceContext->IASetVertexBuffers(0, 2, &pBuffers[0], &pStrides[0], &pOffsets[0]);
}

void Renderer::BindVertexBuffer(int slots, VertexBuffer** vbo)
{
	ID3D11Buffer* pBuffers[ENGINE_MAX_INPUT_SLOTS] = {};
	UINT          pStrides[ENGINE_MAX_INPUT_SLOTS] = {};
	UINT          pOffsets[ENGINE_MAX_INPUT_SLOTS] = {};
	for (int i = 0; i < slots; i++)
	{
		pBuffers[i] = vbo[i]->m_buffer;
		pStrides[i] = vbo[i]->GetStride();
	}

	m_deviceContext->IASetVertexBuffers(0, slots, &pBuffers[0], &pStrides[0], &pOffsets[0]);
}

IndexBuffer* Renderer::CreateIndexBuffer(const size_t size)
{
	IndexBuffer* buffer = new IndexBuffer(size);

	HRESULT result = {};

	D3D11_BUFFER_DESC paramBufferDesc = {};
	paramBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paramBufferDesc.ByteWidth = (UINT)size;
	paramBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	paramBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	result = m_device->CreateBuffer(&paramBufferDesc, nullptr, &buffer->m_buffer);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateBuffer");

	return buffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo)
{
	if (size == 0)
	{
		return;
	}

	HRESULT result = {};

	if (ibo->m_size < size)
	{
		DX_SAFE_RELEASE(ibo->m_buffer);
		// size_t newBufferSize = ibo->m_size + (ibo->m_size >> 1); // grow by half
		IndexBuffer* newibo = CreateIndexBuffer(size); // size > newBufferSize ? size : newBufferSize);
		ibo->m_buffer = newibo->m_buffer;
		ibo->m_size = newibo->m_size;
		newibo->m_buffer = nullptr;
		delete newibo;
	}

	D3D11_MAPPED_SUBRESOURCE paramBufferSource = {};
	result = m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &paramBufferSource);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11DeviceContext::Map");
	memcpy(paramBufferSource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

void Renderer::BindIndexBuffer(const IndexBuffer* ibo)
{
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
}

ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	ConstantBuffer* buffer = new ConstantBuffer(size);

	HRESULT result = {};

	D3D11_BUFFER_DESC paramBufferDesc = {};
	paramBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paramBufferDesc.ByteWidth = (UINT) size;
	paramBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paramBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	result = m_device->CreateBuffer(&paramBufferDesc, nullptr, &buffer->m_buffer);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateBuffer");

	return buffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo)
{
	if (size == 0)
	{
		return;
	}

	HRESULT result = {};

	if (cbo->m_size < size)
	{
		DX_SAFE_RELEASE(cbo->m_buffer);
		ConstantBuffer* newcbo = CreateConstantBuffer(size); // constant buffer size should mostly be constant
		cbo->m_buffer = newcbo->m_buffer;
		cbo->m_size = newcbo->m_size;
		newcbo->m_buffer = nullptr;
		delete newcbo;
	}

	D3D11_MAPPED_SUBRESOURCE paramBufferSource = {};
	result = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &paramBufferSource);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11DeviceContext::Map");
	memcpy(paramBufferSource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

void Renderer::BindConstantBuffer(int slot, const ConstantBuffer* cbo)
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

//------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension)
{
	// See if we already have this font previously loaded
	BitmapFont* existingFont = GetBitmapFontForFileName(bitmapFontFilePathWithNoExtension);
	if (existingFont)
	{
		return existingFont;
	}

	// Never seen this font before!  Let's load it.
	BitmapFont* newFont = CreateBitmapFontFromFile(bitmapFontFilePathWithNoExtension);
	return newFont;
}


//------------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateBitmapFontFromFile(const char* fontFilePathNameWithNoExtension)
{
	std::string fontFilePath = fontFilePathNameWithNoExtension;
	std::string fontTexturePath = fontFilePath + ".png";
	Texture* texture = CreateOrGetTextureFromFile(fontTexturePath.c_str());

	BitmapFont* font = new BitmapFont(fontFilePathNameWithNoExtension, *texture);
	m_loadedBitmapFonts.push_back(font);
	return font;
}

void Renderer::SetDebugName(ID3D11DeviceChild* object, const char* name)
{
#ifdef ENGINE_DEBUG_RENDER
	if (!name)
	{
		ERROR_RECOVERABLE("Setting null name to a d3d object!");
	}
	else
	{
		object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
	}
#endif
}

int GetD3DConstant(CullMode cullMode)
{
	switch (cullMode)
	{
	case CullMode::BACK  : return D3D11_CULL_BACK;
	case CullMode::FRONT : return D3D11_CULL_FRONT;
	case CullMode::NONE  : return D3D11_CULL_NONE;
	}
	ERROR_AND_DIE("No coresponding enum value for CullMode!");
}

int GetD3DConstant(FillMode fillMode)
{
	switch (fillMode)
	{
	case FillMode::SOLID     : return D3D11_FILL_SOLID;
	case FillMode::WIREFRAME : return D3D11_FILL_WIREFRAME;
	}
	ERROR_AND_DIE("No coresponding enum value for FillMode!");
}

int GetD3DConstant(WindingOrder windingOrder)
{
	switch (windingOrder)
	{
	case WindingOrder::CLOCKWISE        : return false;
	case WindingOrder::COUNTERCLOCKWISE : return true;
	}
	ERROR_AND_DIE("No coresponding enum value for WindingOrder!");
}

int GetD3DConstant(DepthTest depthTest)
{
	switch (depthTest)
	{
	case DepthTest::ALWAYS      : return D3D11_COMPARISON_ALWAYS;
	case DepthTest::NEVER       : return D3D11_COMPARISON_NEVER;
	case DepthTest::EQUAL       : return D3D11_COMPARISON_EQUAL;
	case DepthTest::NOTEQUAL    : return D3D11_COMPARISON_NOT_EQUAL; 
	case DepthTest::LESS        : return D3D11_COMPARISON_LESS;
	case DepthTest::LESSEQUAL   : return D3D11_COMPARISON_LESS_EQUAL; 
	case DepthTest::GREATER     : return D3D11_COMPARISON_GREATER;
	case DepthTest::GREATEREQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
	}
	ERROR_AND_DIE("No coresponding enum value for DepthTest!");
}

int GetD3DConstant(ETextureFormat format)
{
    switch (format)
    {
    case ETextureFormat::R8G8B8A8_UNORM:           return DXGI_FORMAT_R8G8B8A8_UNORM;
    case ETextureFormat::D24_UNORM_S8_UINT:        return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case ETextureFormat::R24G8_TYPELESS:           return DXGI_FORMAT_R24G8_TYPELESS;
    case ETextureFormat::R24_UNORM_X8_TYPELESS:    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case ETextureFormat::R32_FLOAT:                return DXGI_FORMAT_R32_FLOAT;
    }
    ERROR_AND_DIE("No coresponding enum value for ETextureFormat!");
}

int GetD3DConstant(EMemoryHint hint)
{
    switch (hint)
    {
    case EMemoryHint::STATIC:     return D3D11_USAGE_DEFAULT;
    case EMemoryHint::GPU:        return D3D11_USAGE_IMMUTABLE;
    case EMemoryHint::DYNAMIC:    return D3D11_USAGE_DYNAMIC;
    // case EMemoryHint::STAGING:    return D3D11_USAGE_STAGING;
    }
    ERROR_AND_DIE("No coresponding enum value for EMemoryHint!");
}

int GetD3DConstant(ETextureBindFlagBit bit)
{
    switch (bit)
    {
    case TEXTURE_BIND_SHADER_RESOURCE_BIT:     return D3D11_BIND_SHADER_RESOURCE;
    case TEXTURE_BIND_RENDER_TARGET_BIT:       return D3D11_BIND_RENDER_TARGET;
    case TEXTURE_BIND_DEPTH_STENCIL_BIT:       return D3D11_BIND_DEPTH_STENCIL;
    }
    ERROR_AND_DIE("No coresponding enum value for ETextureBindFlagBit!");
}

int GetD3DFlags(ETextureBindFlags flags)
{
	int flagsResult = 0;
	
    static constexpr ETextureBindFlagBit values[3]
	{
	    TEXTURE_BIND_SHADER_RESOURCE_BIT,
        TEXTURE_BIND_RENDER_TARGET_BIT,
        TEXTURE_BIND_DEPTH_STENCIL_BIT,
	};

	for (auto& bit : values)
		flagsResult |= ((flags & bit) == bit) ? GetD3DConstant(bit) : 0;

	return flagsResult;
}

void RasterizerState::Release()
{
	DX_SAFE_RELEASE(m_d3dRasterizerState);
}

void DepthStencilState::Release()
{
	DX_SAFE_RELEASE(m_depthStencilState);
	DX_SAFE_RELEASE(m_depthStencilView);
}

void BlendState::Release()
{
	for (auto& blendState : m_blendStates)
	{
		DX_SAFE_RELEASE(blendState);
	}
}

void SamplerState::Release()
{
	for (auto& samplerState : m_samplerStates)
	{
		DX_SAFE_RELEASE(samplerState);
	}
}

#endif