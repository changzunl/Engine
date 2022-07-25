#pragma once

#define ENGINE_MAX_INPUT_SLOTS 16
#define ENGINE_MAX_TEXTURE_SLOTS 8

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat4x4.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RendererConfig.hpp"
#include "Engine/Renderer/RenderStates.hpp"
#include "Engine/Renderer/VertexFormat.hpp"

#include "Game/EngineBuildPreferences.hpp"

#include <vector>
#include <string>

struct RgbaF;
struct IntVec2;
class  Texture;
class  BitmapFont;
class  Shader;
class  VertexBuffer;
class  IndexBuffer;
class  ConstantBuffer;
class  Image;
class  Renderer;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11DeviceChild;

struct ModelConstants
{
    Mat4x4 ModelMatrix;
    Vec4   TintColor;
};

struct CameraConstants
{
    Mat4x4 ProjectionMatrix;
    Mat4x4 ViewMatrix;
};

struct LightConstants
{
	Vec3 SunDirection = Vec3(0.0f, 0.0f, -1.0f);
	float SunIntensity = 1.0f;
	Vec3 SunColor = Vec3(1.0f, 1.0f, 1.0f);
	float AmbientIntensity = 0.1f;
};

constexpr int LIGHT_CONSTANT_BUFFER_SLOT = 1;
constexpr int CAMERA_CONSTANT_BUFFER_SLOT = 2;
constexpr int MODEL_CONSTANT_BUFFER_SLOT  = 3;
constexpr int CUSTOM_CONSTANT_BUFFER_SLOT_START = 4;
constexpr int CUSTOM_CONSTANT_BUFFER_SLOT_SIZE = 4;

typedef std::vector<Vertex_PCU> VertexList;
typedef std::vector<ConstantBuffer*> CBOList;

class Renderer
{
public:
    Renderer(const RendererConfig& theConfig);
    ~Renderer();

    // lifecycle
    void               Startup();
    void               BeginFrame();
    void               EndFrame();
    void               Shutdown();
                       
    // Camera & render
    const AABB2&       GetViewport();
    void               SetViewport(const AABB2& viewport);
    void               ClearScreen(const Rgba8& color);
    void               ClearScreen(const Rgba8& color, Texture* renderTarget);
    void               ClearDepth(float value = 1.0f);
    ModelConstants&    GetModelConstants();
    LightConstants&    GetLightConstants();
    void               UpdateLights();
    void               SetTintColor(const Rgba8& color);
    void               SetModelMatrix(const Mat4x4& modelMatrix);
    void               BeginCamera(const Camera& camera);
    void               EndCamera(const Camera& camera);
    void               DrawVertexArray(VertexList vertices);
    void               DrawVertexArray(int numVertex, const Vertex_PCU* vertexArray);
    void               DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0);
    void               DrawVertexBuffer(int slots, VertexBuffer** vbo, int vertexCount, int vertexOffset = 0);
	void               DrawIndexedVertexBuffer(IndexBuffer* ibo, VertexBuffer* vbo, int indexCount, int indexOffset = 0, int vertexOffset = 0);
	void               DrawIndexedVertexBuffer(IndexBuffer* ibo, int slots, VertexBuffer** vbo, int indexCount, int indexOffset = 0, int vertexOffset = 0);
	void               DrawInstancedVertexBuffer(VertexBuffer** vbo, int vertexCount, int instanceCount, int vertexOffset = 0, int instanceOffset = 0);
	void               DrawInstancedVertexBuffer(int slots, VertexBuffer** vbo, int vertexCount, int instanceCount, int vertexOffset = 0, int instanceOffset = 0);
	void               DrawIndexedInstancedVertexBuffer(IndexBuffer* ibo, VertexBuffer** vbo, int indexCount, int instanceCount, int indexOffset = 0, int vertexOffset = 0, int instanceOffset = 0);
	void               DrawIndexedInstancedVertexBuffer(int slots, IndexBuffer* ibo, VertexBuffer** vbo, int indexCount, int instanceCount, int indexOffset = 0, int vertexOffset = 0, int instanceOffset = 0);

    // render states
	void               SetCullMode(CullMode cullMode);
	void               SetFillMode(FillMode fillMode);
	void               SetWindingOrder(WindingOrder windingOrder);
	void               SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder);
	void               SetDepthTest(DepthTest depthTest);
	void               SetDepthMask(bool writeDepth);
	void               SetDepthStencilState(DepthTest depthTest, bool writeDepth);
	void               SetBlendMode(BlendMode blendMode);
	void               SetSamplerMode(SamplerMode samplerMode);
    void               InitializeCustomConstantBuffer(int slot, size_t size);
    void               SetCustomConstantBuffer(int slot, const void* data);
                       
	// Textures        
    Texture*           GetScreenTexture();
	void               SetDepthTarget(Texture* texture);
	void               SetRenderTargets(int size, Texture* const* textures);
    Texture*           CreateTexture(IntVec2 dimensions, bool isR32 = false, bool isDepth = false, bool isTarget = false);
    void               DeleteTexture(Texture*& texture);
    Texture*           CreateOrGetTextureFromFile(const char* imageFilePath);
    Texture*           GetDepthStencilTexture();
    BitmapFont*        CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension);
    void               BindTexture(const Texture* texture, int index = 0);
                       
    // Shaders         
    const VertexFormat& GetDefaultVF_PCU();
    const VertexFormat& GetDefaultVF_PNCU();
    Shader*            CreateOrGetShader(const char* shaderName);
    void               BindShaderByName(const char* shaderName);
    void               BindShader(Shader* shader);
    Shader*            GetShaderForName(const char* shaderName);
    Shader*            CreateShaderFromFile(const char* shaderName);
    Shader*            CreateShader(const char* shaderName, std::string& shaderSource, const VertexFormat& vertexFormat);
    Shader*            CreateShader(const char* shaderName, std::string& shaderSource, const std::vector<VertexFormat>& vertexFormats);
    bool               CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, const char* name, std::string& source, const char* entryPoint, const char* target);
                       
    // Buffers         
    VertexBuffer*      CreateVertexBuffer(const size_t size, const int stride = sizeof(Vertex_PCU));
    VertexBuffer*      CreateVertexBuffer(const size_t size, const VertexFormat* format);
    VertexBuffer*      CreateInstanceBuffer(const size_t size, const VertexFormat* format);
    void               CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo);
    void               BindVertexBuffer(const VertexBuffer* vbo);
    void               BindVertexBuffer(int slots, VertexBuffer** vbo);
    void               BindVertexBuffer(const VertexBuffer* vertexBuffer, const VertexBuffer* instanceBuffer);
    IndexBuffer*       CreateIndexBuffer(const size_t size);
    void               CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo);
    void               BindIndexBuffer(const IndexBuffer* ibo);
	ConstantBuffer*    CreateConstantBuffer(const size_t size);
	void               CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo);
	void               BindConstantBuffer(int slot, const ConstantBuffer* cbo);

protected:
    void               CreateRenderContext();
	void               ReleaseRenderContext();
	void               CreateRenderState();

    void               CreateDepthState();
    void               CreateBlendState();
	void               CreateSamplerState();
	void               InitializeRenderState();
	void               ReleaseRenderState();
    void               SetRenderStates();
	void               SetModelState();
	void               SetLightState();
    Texture*           CreateTextureFromFile(const char* imageFilePath);
    Texture*           CreateTextureFromImage(const Image& image);
    Texture*           GetTextureForFileName(const char* imageFilePath);
    BitmapFont*        CreateBitmapFontFromFile(const char* fontFilePathNameWithNoExtension);
    BitmapFont*        GetBitmapFontForFileName(const char* fontFilePathNameWithNoExtension);
    void               SetDebugName(ID3D11DeviceChild* object, const char* name);

protected:
    RendererConfig              m_theConfig;

    ID3D11Device*               m_device                            = nullptr;
    ID3D11DeviceContext*        m_deviceContext                     = nullptr;
    IDXGISwapChain*             m_swapChain                         = nullptr; 
    Texture*                    m_renderTargetScreen                = nullptr;
    Texture*                    m_depthTargetTexture                = nullptr;
    ID3D11DepthStencilView*     m_currentDepthTarget                = nullptr;

#ifdef ENGINE_DEBUG_RENDER
    void*                       m_dxgiDebugModule                   = nullptr;
    void*                       m_dxgiDebug                         = nullptr;
#endif

    std::vector<BitmapFont*>    m_loadedBitmapFonts;

    std::vector<Texture*>       m_loadedTextures;
    Texture*                    m_defaultTexture                        = nullptr;
    Texture*                    m_currentTexture[ENGINE_MAX_TEXTURE_SLOTS] = {};

    VertexFormat                m_defaultVF_PCU;
    VertexFormat                m_defaultVF_PNCU;
    std::vector<Shader*>        m_loadedShaders;                        
    Shader*                     m_defaultShader                         = nullptr;
    Shader*                     m_currentShader                         = nullptr;

    ConstantBuffer*             m_modelCBO                              = nullptr;
    ConstantBuffer*             m_cameraCBO                             = nullptr;
    ConstantBuffer*             m_lightCBO                              = nullptr;
    ConstantBuffer*             m_customCBOs[CUSTOM_CONSTANT_BUFFER_SLOT_SIZE];
    VertexBuffer*               m_immediateVBO                          = nullptr;

    AABB2                       m_viewport                              = AABB2::ZERO_TO_ONE;
    LightConstants              m_lightConstants;
    ModelConstants              m_modelConstants;
    RasterizerState             m_rasterizerState;                      
    DepthStencilState           m_depthStencilState;                    
    BlendState                  m_blendState;                    
    SamplerState                m_samplerState;        

#ifdef ENGINE_USE_SOFTWARE_RASTERIZER

private:
    void               TransformVertex(Vec3& inout_vertPos, float& out_w, bool& out_clip) const;
    void               RasterizeTriangle(const Vertex_PCU* verts, const float* vertsW);
    const Rgba8&       SampleTexture2D(const Texture* texture, Vec2 texCoord);
    void               WritePixel(size_t x, size_t y, Vec2 texCoord, const RgbaF& tintColor, float depth);
    
private:
    size_t                 m_screenDimensionX       = 0;
    size_t                 m_screenDimensionY       = 0;
    Rgba8*                 m_screenBuffer           = nullptr;
    float*                 m_depthBuffer            = nullptr;
    CameraConstants        m_cameraConstants;

    const VertexBuffer*    m_vbo                    = nullptr;
    const IndexBuffer*     m_ibo                    = nullptr;

#endif
};

