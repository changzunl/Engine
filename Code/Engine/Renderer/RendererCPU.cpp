#include "Renderer.hpp"

#ifdef ENGINE_USE_SOFTWARE_RASTERIZER

#include "Engine/Core/RgbaF.hpp"
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
#include <ThirdParty/stb/stb_image.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#undef OPAQUE

#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

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
	// DX11 init
	HRESULT result = {};

	// DX11 device initialize
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

	UINT paramSDKVersion = D3D11_SDK_VERSION;
	D3D_DRIVER_TYPE paramDriverType = D3D_DRIVER_TYPE_HARDWARE;
	UINT paramFlags = 0;
	D3D_FEATURE_LEVEL paramFeatureLevel = D3D_FEATURE_LEVEL_12_1;
#ifdef ENGINE_DEBUG_RENDER
	paramFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	result = ::D3D11CreateDeviceAndSwapChain(nullptr, paramDriverType, 0, paramFlags, 0, 0, paramSDKVersion, &paramSwapChainDesc, &m_swapChain, &m_device, &paramFeatureLevel, &m_deviceContext);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ::D3D11CreateDeviceAndSwapChain");

	// software rasterization resource initialize
	const IntVec2& screenDimension = m_theConfig.m_window->GetClientDimensions();
	m_screenDimensionX             = screenDimension.x;
	m_screenDimensionY             = screenDimension.y;
	m_screenBuffer                 = new Rgba8[m_screenDimensionX * m_screenDimensionY];
	m_depthBuffer                  = new float[m_screenDimensionX * m_screenDimensionY];
}

void Renderer::CreateRenderState()
{
	HRESULT result = {};

	// init viewport
	UINT paramNumViewports = 1;
	D3D11_VIEWPORT paramViewPort = {};
	paramViewPort.TopLeftX = 0.0f;
	paramViewPort.TopLeftY = 0.0f;
	paramViewPort.Width = (float)m_theConfig.m_window->GetClientDimensions().x;
	paramViewPort.Height = (float)m_theConfig.m_window->GetClientDimensions().y;
	paramViewPort.MinDepth = 0.0f;
	paramViewPort.MaxDepth = 1.0f;

	m_deviceContext->RSSetViewports(paramNumViewports, &paramViewPort);

	// create depth stencil state
	D3D11_TEXTURE2D_DESC pTextureDesc = {};
	pTextureDesc.Width = m_theConfig.m_window->GetClientDimensions().x;
	pTextureDesc.Height = m_theConfig.m_window->GetClientDimensions().y;
	pTextureDesc.MipLevels = 1;
	pTextureDesc.ArraySize = 1;
	pTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	pTextureDesc.SampleDesc.Count = 1;
	pTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	pTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	result = m_device->CreateTexture2D(&pTextureDesc, nullptr, &m_depthStencilState.m_depthStencilTexture);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call ID3D11Device::CreateTexture2D");

	// create blend state
	m_blendState.m_blendMode = (BlendMode)-1;

	// create sampler state
	m_samplerState.m_samplerMode = (SamplerMode)-1;

	// create shader cbo

	// create default vbo

	// create default texture
	m_defaultTexture = CreateTextureFromImage(Image::WHITE_PIXEL);

	// create default shader
	m_defaultShader = CreateShader("Default", SHADER_SOURCE_EMBEDDED);
}

void Renderer::CreateBlendState()
{
}

void Renderer::CreateSamplerState()
{
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
	m_blendState.SetBlendMode(BlendMode::ALPHA);
	BindShader(nullptr);
	BindTexture(nullptr);
	SetTintColor(Rgba8::WHITE);

	DebugRenderBeginFrame();
}

void Renderer::EndFrame()
{
	DebugRenderEndFrame();

	HRESULT result = {};

	ID3D11Texture2D* pTexture2D = nullptr;

	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pTexture2D);
	ASSERT_OR_DIE(SUCCEEDED(result), "Failed to call IDXGISwapChain::GetBuffer");

	m_deviceContext->UpdateSubresource(pTexture2D, 0, NULL, m_screenBuffer, (int) m_screenDimensionX * sizeof(Rgba8), (int)m_screenDimensionX * (int)m_screenDimensionY * sizeof(Rgba8));

	DX_SAFE_RELEASE(pTexture2D);

#ifdef ENGINE_DEBUG_RENDER
	// Debug write depth buffer to GPU for RenderDoc inspection
	m_deviceContext->UpdateSubresource(m_depthStencilState.m_depthStencilTexture, 0, NULL, m_depthBuffer, (int)m_screenDimensionX * sizeof(float), (int)m_screenDimensionX * (int)m_screenDimensionY * sizeof(float));
#endif

	// "Present" the back buffer by swapping the front (visible) and back (working) screen buffers
	m_swapChain->Present(0, 0);
}

void Renderer::Shutdown()
{
	delete[] m_screenBuffer;
	delete[] m_depthBuffer;
	m_screenDimensionX = 0;
	m_screenDimensionY = 0;
	m_screenBuffer     = nullptr;
	m_depthBuffer      = nullptr;

	DebugRenderSystemShutdown();

	for (Texture* texture : m_loadedTextures)
	{
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

void Renderer::ReleaseRenderState()
{
	// delete default shader
	delete m_defaultShader;
	m_defaultShader = nullptr;

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

	m_blendState.Release();
	m_samplerState.Release();
	m_rasterizerState.Release();
	m_depthStencilState.Release();
}

void Renderer::SetRenderStates()
{
}

void Renderer::ReleaseRenderContext()
{
	DX_SAFE_RELEASE(m_renderTarget);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);
}

void Renderer::ClearScreen(const Rgba8& color)
{
	for (size_t i = 0; i < m_screenDimensionY * m_screenDimensionX; i++)
	{
		m_screenBuffer[i] = color;
	}

	ClearDepth();
}

void Renderer::ClearDepth(float value /*= 1.0f*/)
{
	for (size_t i = 0; i < m_screenDimensionY * m_screenDimensionX; i++)
	{
		m_depthBuffer[i] = value;
	}
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
}

void Renderer::BeginCamera(const Camera& camera)
{
	m_cameraConstants.ProjectionMatrix = camera.GetProjectionMatrix();
	m_cameraConstants.ProjectionMatrix.Append(camera.GetRenderMatrix().GetOrthonormalInverse());
	m_cameraConstants.ViewMatrix = camera.GetViewMatrix();

	m_modelConstants.ModelMatrix.SetIdentity();
	m_modelConstants.TintColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Renderer::EndCamera(const Camera& camera)
{
	UNUSED(camera);
}

void Renderer::DrawVertexArray(int numVertex, const Vertex_PCU* vertexArray)
{
	if (m_rasterizerState.m_fillMode != FillMode::SOLID)
		return; // wire frame is not supported yet

	Vertex_PCU verts[3] = {};
	float vertsW[3] = {};
	bool clip[3] = {};
	for (int i = 0; i < numVertex; i += 3)
	{
		verts[0] = vertexArray[i + 0];
		verts[1] = vertexArray[i + 1];
		verts[2] = vertexArray[i + 2];
		TransformVertex(verts[0].m_position, vertsW[0], clip[0]);
		TransformVertex(verts[1].m_position, vertsW[1], clip[1]);
		TransformVertex(verts[2].m_position, vertsW[2], clip[2]);

		// frustum clipping
		if (clip[0] && clip[1] && clip[2])
			continue;

		RasterizeTriangle(&verts[0], &vertsW[0]);
	}
}

void Renderer::DrawVertexArray(VertexList vertices)
{
	DrawVertexArray((int) vertices.size(), vertices.data());
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0*/)
{
	DrawVertexArray(vertexCount, &((Vertex_PCU*)vbo->m_buffer)[vertexOffset]);
}

void Renderer::DrawIndexedVertexBuffer(IndexBuffer* ibo, VertexBuffer* vbo, int vertexCount, int indexOffset, int vertexOffset)
{
	if (m_rasterizerState.m_fillMode != FillMode::SOLID)
		return; // wire frame is not supported yet

	Vertex_PCU*   vertexArray = (Vertex_PCU*)vbo->m_buffer;
	unsigned int* indexArray  = (unsigned int*)ibo->m_buffer;

	Vertex_PCU verts[3] = {};
	float vertsW[3] = {};
	bool clip[3] = {};
	for (int i = 0; i < vertexCount; i += 3)
	{
		verts[0] = vertexArray[vertexOffset + indexArray[indexOffset + i]];
		verts[1] = vertexArray[vertexOffset + indexArray[indexOffset + i + 1]];
		verts[2] = vertexArray[vertexOffset + indexArray[indexOffset + i + 2]];
		TransformVertex(verts[0].m_position, vertsW[0], clip[0]);
		TransformVertex(verts[1].m_position, vertsW[1], clip[1]);
		TransformVertex(verts[2].m_position, vertsW[2], clip[2]);

		// frustum clipping
		if (clip[0] && clip[1] && clip[2])
			continue;

		RasterizeTriangle(&verts[0], &vertsW[0]);
	}
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
	size_t size = (size_t)image.GetDimensions().x * (size_t)image.GetDimensions().y * sizeof(Rgba8);
	char* data = new char[size];
	memcpy(data, image.GetRawData(), size);

	Texture* texture = new Texture();
	texture->m_name = image.GetImageFilePath();
	texture->m_dimensions = image.GetDimensions();
	texture->m_texture = (ID3D11Texture2D*)data;

	return texture;
}


//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture(const Texture* texture)
{
	if (!texture)
		texture = m_defaultTexture;
	if (texture != m_currentTexture)
	{
		m_currentTexture = (Texture*)texture;
	}
}


Shader* Renderer::CreateOrGetShader(const char* shaderName)
{
	Shader* shader = GetShaderForName(shaderName);
	if (!shader)
	{
		std::string fileName = std::string("Data/Shaders/") + shaderName + std::string(".hlsl");
		std::string source;
		if (FileReadToString(source, fileName) < 0)
			ERROR_AND_DIE(Stringf("Shader source file not found: ", fileName.c_str()));

		shader = CreateShader(shaderName, source);
		m_loadedShaders.push_back(shader);
	}
	return shader;
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
}

Shader* Renderer::GetShaderForName(const char* shaderName)
{
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

Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource)
{
	std::string sourceStr = shaderSource;
	return CreateShader(shaderName, sourceStr);
}

Shader* Renderer::CreateShader(const char* shaderName, std::string& shaderSource)
{
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;

	Shader* shader = new Shader(shaderConfig);

	UNUSED(shaderSource);

	return shader;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, const char* name, std::string& source, const char* entryPoint, const char* target)
{
	UNUSED(outByteCode);
	UNUSED(name);
	UNUSED(source);
	UNUSED(entryPoint);
	UNUSED(target);
	return true;
}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size)
{
	VertexBuffer* buffer = new VertexBuffer(size);
	if (size)
		buffer->m_buffer = (ID3D11Buffer*)new char[size];
	return buffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo)
{
	if (vbo->m_size < size)
	{
		delete[] (char*)vbo->m_buffer;
		vbo->m_size = size;
		vbo->m_buffer = (ID3D11Buffer*)new char[size];
	}
	memcpy(vbo->m_buffer, data, size);
}

void Renderer::BindVertexBuffer(const VertexBuffer* vbo)
{
	m_vbo = vbo;
}

IndexBuffer* Renderer::CreateIndexBuffer(const size_t size)
{
	IndexBuffer* buffer = new IndexBuffer(size);
	if (size)
		buffer->m_buffer = (ID3D11Buffer*)new char[size];
	return buffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo)
{
	if (ibo->m_size < size)
	{
		delete[](char*)ibo->m_buffer;
		ibo->m_size = size;
		ibo->m_buffer = (ID3D11Buffer*)new char[size];
	}
	memcpy(ibo->m_buffer, data, size);
}

void Renderer::BindIndexBuffer(const IndexBuffer* ibo)
{
	m_ibo = ibo;
}

ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	ConstantBuffer* buffer = new ConstantBuffer(size);
	if (size)
		buffer->m_buffer = (ID3D11Buffer*)new char[size];
	return buffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo)
{
	if (cbo->m_size < size)
	{
		delete[](char*)cbo->m_buffer;
		cbo->m_size = size;
		cbo->m_buffer = (ID3D11Buffer*)new char[size];
	}
	memcpy(cbo->m_buffer, data, size);
}

void Renderer::BindConstantBuffer(int slot, const ConstantBuffer* cbo)
{
	switch (slot)
	{
	case CAMERA_CONSTANT_BUFFER_SLOT:
		m_cameraConstants = *(CameraConstants*)cbo->m_buffer;
		break;
	case MODEL_CONSTANT_BUFFER_SLOT:
		m_modelConstants = *(ModelConstants*)cbo->m_buffer;
		break;
	default:
		DebuggerPrintf(Stringf("Unknown constant slot: %d", slot).c_str());
	}
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
	UNUSED(object);
	UNUSED(name);
}

void Renderer::TransformVertex(Vec3& inout_vertPos, float& out_w, bool& out_clip) const
{
	// vertex transform

	// vertex shader begin
	Vec4 localPos = Vec4(inout_vertPos.x, inout_vertPos.y, inout_vertPos.z, 1.0f);
	Vec4 worldPos = m_modelConstants.ModelMatrix.TransformHomogeneous3D(localPos);
	Vec4 screenPos = m_cameraConstants.ProjectionMatrix.TransformHomogeneous3D(m_cameraConstants.ViewMatrix.TransformHomogeneous3D(worldPos));
	// vertex shader end

	// homogeneous correction
	out_w = screenPos.w;
	if (screenPos.w != 1.0f)
	{
		float dW = 1.0f / screenPos.w;
		screenPos.x *= dW;
		screenPos.y *= dW;
		screenPos.z *= dW;
	}

	// clipping
	out_clip = !(
		//(screenPos.z >=  0.0f && screenPos.z <= 1.0f) &&
		(screenPos.x >= -1.0f && screenPos.x <= 1.0f) &&
		(screenPos.y >= -1.0f && screenPos.y <= 1.0f));

	// NDC to actual screen
	inout_vertPos.x = (screenPos.x + 1.0f) * 0.5f * (float)m_screenDimensionX;
	inout_vertPos.y = (1.0f - screenPos.y) * 0.5f * (float)m_screenDimensionY;
	inout_vertPos.z = screenPos.z;
}

void CalcBarycentricRasterize(const Vertex_PCU* vertices, const Vec3& point, float xTriangleInv, Vec3& bcCoords)
{
	float xPB = (vertices[2].m_position - vertices[1].m_position).Cross2D(point - vertices[1].m_position);
	float xPC = (vertices[0].m_position - vertices[2].m_position).Cross2D(point - vertices[2].m_position);

	bcCoords.x = xPB * xTriangleInv;
	bcCoords.y = xPC * xTriangleInv;
	bcCoords.z = 1.0f - bcCoords.x - bcCoords.y;
}

void Renderer::RasterizeTriangle(const Vertex_PCU* verts, const float* vertsW)
{
	// box
	IntVec2 minCoords;
	IntVec2 maxCoords;
	minCoords.x = Floor(Min(verts[0].m_position.x, Min(verts[1].m_position.x, verts[2].m_position.x)));
	minCoords.y = Floor(Min(verts[0].m_position.y, Min(verts[1].m_position.y, verts[2].m_position.y)));
	maxCoords.x = Ceil (Max(verts[0].m_position.x, Max(verts[1].m_position.x, verts[2].m_position.x)));
	maxCoords.y = Ceil (Max(verts[0].m_position.y, Max(verts[1].m_position.y, verts[2].m_position.y)));

	// cull
	float cullSide = -1.0f;
	float xBA = 0.0f;
	switch (m_rasterizerState.m_cullMode)
	{
	case CullMode::FRONT:
		cullSide = 1.0f;
		// fall through
	case CullMode::BACK:
		xBA = (verts[2].m_position - verts[1].m_position).Cross2D(verts[1].m_position - verts[0].m_position) * cullSide;
		if (signbit(xBA) != (m_rasterizerState.m_windingOrder == WindingOrder::COUNTERCLOCKWISE ? true : false))
			return;
		break;
	case CullMode::NONE:
		break;
	}

	// pixel shader begin (early calculate)
	// early color
	RgbaF       modelColor = *(RgbaF*)&m_modelConstants.TintColor;
	RgbaF       vertColor  = verts->m_color; // flat shading
	RgbaF       tintColor  = modelColor * vertColor;
	
	// early alpha test
	if (tintColor.a <= 0.0f)
		return;
	// pixel shader end (early calculate)

	float xTriangleInv = 1.0f / (verts[1].m_position - verts[0].m_position).Cross2D(verts[2].m_position - verts[0].m_position);

	// rasterize
#ifndef ENGINE_SOFTWARE_RASTERIZATION_HALF_RES
	constexpr int step = 1;
#else
	constexpr int step = 2;
#endif

	Vec3 screenPos;
	Vec3 bcCoord;
	Vec2 texCoord;
	for (int y = MaxInt(minCoords.y, 0); y <= maxCoords.y && y < m_screenDimensionY; y += step)
	{
		for (int x = MaxInt(minCoords.x, 0); x <= maxCoords.x && x < m_screenDimensionX; x += step)
		{
			screenPos.x = ((float)x) + 0.5f;
			screenPos.y = ((float)y) + 0.5f;
			CalcBarycentricRasterize(verts, screenPos, xTriangleInv, bcCoord);

			if (bcCoord.x < 0.0f || bcCoord.y < 0.0f || bcCoord.z < 0.0f)
				continue;

			screenPos.z = 1.0f / (bcCoord.x / verts[0].m_position.z + bcCoord.y / verts[1].m_position.z + bcCoord.z / verts[2].m_position.z);
			if (screenPos.z > 1.0f || screenPos.z < 0.0f)
				continue;

			bcCoord /= *(Vec3*)vertsW;
			bcCoord /= (bcCoord.x + bcCoord.y + bcCoord.z);

			texCoord.x = bcCoord.x * verts[0].m_uvTexCoords.x + bcCoord.y * verts[1].m_uvTexCoords.x + bcCoord.z * verts[2].m_uvTexCoords.x;
			texCoord.y = bcCoord.x * verts[0].m_uvTexCoords.y + bcCoord.y * verts[1].m_uvTexCoords.y + bcCoord.z * verts[2].m_uvTexCoords.y;

			WritePixel(x, y, texCoord, tintColor, screenPos.z);
		}
	}
}

const Rgba8& Renderer::SampleTexture2D(const Texture* texture, Vec2 texCoord)
{
	const int& width  = texture->GetDimensions().x;
	const int& height = texture->GetDimensions().y;
	
	int x = (int)floorf(texCoord.x * (width));
	int y = (int)floorf(texCoord.y * (height));

	// bilinear not implemented
	switch (m_samplerState.m_samplerMode)
	{
	case SamplerMode::BILINEARCLAMP:
	case SamplerMode::POINTCLAMP:
		ClampIntValue(x, 0, width - 1);
		ClampIntValue(y, 0, height - 1);
		break;
	case SamplerMode::BILINEARWRAP:
	case SamplerMode::POINTWRAP:
		x %= width;
		y %= height;
		break;
	}
	
	return ((Rgba8*)texture->m_texture)[x + y * width];
}

void Renderer::WritePixel(size_t x, size_t y, Vec2 texCoord, const RgbaF& tintColor, float depth)
{
#define discard return

#ifdef ENGINE_SOFTWARE_RASTERIZATION_HALF_RES
 	x -= x % 2;
 	y -= y % 2;
#endif
 	size_t offset  = x + y * m_screenDimensionX;
	
	// depth test
	switch (m_depthStencilState.m_depthTest)
	{
	case DepthTest::ALWAYS:
		break;
	case DepthTest::NEVER:
		discard;
	case DepthTest::EQUAL:
		if (depth == m_depthBuffer[offset])
			break;
		discard;
	case DepthTest::GREATER:
		if (depth > m_depthBuffer[offset])
			break;
		discard;
	case DepthTest::GREATEREQUAL:
		if (depth >= m_depthBuffer[offset])
			break;
		discard;
	case DepthTest::LESS:
		if (depth < m_depthBuffer[offset])
			break;
		discard;
	case DepthTest::LESSEQUAL:
		if (depth <= m_depthBuffer[offset])
			break;
		discard;
	default:
		discard;
	}

	// pixel shader begin
	Rgba8 diffuse = SampleTexture2D(m_currentTexture, texCoord);
	Rgba8 srcColor = (tintColor * diffuse).GetAsRgba8();

	if (srcColor.a <= 0)
		discard;
	// pixel shader end

	Rgba8 dstColor = m_screenBuffer[offset];
	float srcAlpha = 0.0f;
	// alpha blend & write pixel
	switch (m_blendState.m_blendMode)
	{
	case BlendMode::ADDITIVE:
		m_screenBuffer[offset].r = (unsigned char)ClampInt((int)srcColor.r + (int)dstColor.r, 0, 255);
		m_screenBuffer[offset].g = (unsigned char)ClampInt((int)srcColor.g + (int)dstColor.g, 0, 255);
		m_screenBuffer[offset].b = (unsigned char)ClampInt((int)srcColor.b + (int)dstColor.b, 0, 255);
		m_screenBuffer[offset].a = (unsigned char)ClampInt((int)srcColor.a + (int)dstColor.a, 0, 255);
		break;
	case BlendMode::ALPHA:
		srcAlpha = NormalizeByte(srcColor.a);
		m_screenBuffer[offset].r = (unsigned char)ClampInt(int(srcAlpha * (float)srcColor.r) + int((1.0f - srcAlpha) * (float)dstColor.r), 0, 255);
		m_screenBuffer[offset].g = (unsigned char)ClampInt(int(srcAlpha * (float)srcColor.g) + int((1.0f - srcAlpha) * (float)dstColor.g), 0, 255);
		m_screenBuffer[offset].b = (unsigned char)ClampInt(int(srcAlpha * (float)srcColor.b) + int((1.0f - srcAlpha) * (float)dstColor.b), 0, 255);
		m_screenBuffer[offset].a = srcColor.a;
		break;
	case BlendMode::OPAQUE:
		m_screenBuffer[offset] = srcColor;
		break;
	default: 
		discard;
	}

#ifdef ENGINE_SOFTWARE_RASTERIZATION_HALF_RES
 	m_screenBuffer[offset + 1] = m_screenBuffer[offset];
 	m_screenBuffer[offset + m_screenDimensionX] = m_screenBuffer[offset];
 	m_screenBuffer[offset + 1 + m_screenDimensionX] = m_screenBuffer[offset];
#endif

	// write depth
	if (m_depthStencilState.m_depthMask)
		m_depthBuffer[offset] = depth;

#undef discard
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
	case DepthTest::NEVER 		: return D3D11_COMPARISON_NEVER;
	case DepthTest::EQUAL 		: return D3D11_COMPARISON_EQUAL;
	case DepthTest::NOTEQUAL 	: return D3D11_COMPARISON_NOT_EQUAL; 
	case DepthTest::LESS 		: return D3D11_COMPARISON_LESS;
	case DepthTest::LESSEQUAL 	: return D3D11_COMPARISON_LESS_EQUAL; 
	case DepthTest::GREATER     : return D3D11_COMPARISON_GREATER;
	case DepthTest::GREATEREQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
	}
	ERROR_AND_DIE("No coresponding enum value for DepthTest!");
}

void RasterizerState::Release()
{
	DX_SAFE_RELEASE(m_d3dRasterizerState);
}

void DepthStencilState::Release()
{
	DX_SAFE_RELEASE(m_depthStencilState);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_depthStencilTexture);
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