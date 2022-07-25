#pragma once

#define ENGINE_VERTEX_FORMAT_MAX_ELEMENTS 16
#define ENGINE_VERTEX_FORMAT_MAX_TEXCOORDS 2

#include <string>
#include <vector>

typedef long HRESULT;
class VertexFormat;
struct D3D11_INPUT_ELEMENT_DESC;

class VertexFormatElement;
class VertexFormat;
class VertexBufferBuilder;

class VertexFormatElement
{
    friend class VertexFormat;

public:
    enum class Format
    {
        UB4_RGBA,
        UB4_BGRA,
        UB4,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        NORM3,
        SIZE,
    };

    enum class Semantic
	{
		POSITION,
		NORMAL,
        COLOR,
        TEXCOORD,
        GENERIC,
    };

    static int GetSize(Format format);
    static int GetD3DConstant(Format format);

    static VertexFormatElement POSITION3F;
    static VertexFormatElement NORMAL3F;
    static VertexFormatElement COLOR4UB;
    static VertexFormatElement TEXCOORD2F;

	VertexFormatElement(std::string name, Format format, Semantic semantic = Semantic::GENERIC);

public:
    std::string    m_name;
    Format         m_format;
    Semantic       m_semantic;
};


class VertexFormat
{
    friend class Renderer;
    friend class VertexBufferBuilder;

public:
    VertexFormat();

    void AddElement(const VertexFormatElement& element);
    int GetVertexStride() const;
	void GetInputElements(std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc) const;

public:
    static VertexFormat GetDefaultFormat_Vertex_PCU();
    static VertexFormat GetDefaultFormat_Vertex_PNCU();

private:
    std::vector<VertexFormatElement> m_elements;
    int m_stride = 0;

public:
    bool m_instanced = false;
    int  m_bufferSlot = 0;
    int  m_posPosition = -1;
    int  m_posNormal = -1;
    int  m_posColor = -1;
    int  m_posUVs[ENGINE_VERTEX_FORMAT_MAX_TEXCOORDS] = { -1 };
    int  m_pos[ENGINE_VERTEX_FORMAT_MAX_ELEMENTS] = { -1 };
};


class Renderer;
class VertexBuffer;
struct Rgba8;
struct RgbaF;
struct Vec2;
struct Vec3;

/*
    To build vertex buffer, call those functions
    1. Call start() to initialize the builder
    2. Call begin() to start build one vertex
    3. Call pos()/normal()/color() etc.. to build each component of a vertex
    4. Call end() to finish a vertex
    5. After all vertex you need is built, call upload() to upload built vertex buffer to gpu
    6. Call reset() to reset the builder and clear the buffer in it
*/
class VertexBufferBuilder
{

public:
    void Start(const VertexFormat& format, int count);

    VertexBufferBuilder* begin();
    VertexBufferBuilder* pos(float x, float y, float z);
    VertexBufferBuilder* pos(const Vec3& position);
    VertexBufferBuilder* normal(float x, float y, float z);
    VertexBufferBuilder* normal(const Vec3& normal);
    VertexBufferBuilder* color(int colorRGBA);
    VertexBufferBuilder* color(const Rgba8& color);
    VertexBufferBuilder* color(const RgbaF& colorF);
    VertexBufferBuilder* color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
    VertexBufferBuilder* tex(float u, float v, int compIdx = 0);
    VertexBufferBuilder* tex(const Vec2& texcoords, int compIdx = 0);
	VertexBufferBuilder* float1(const float* data, int elementIdx);
	VertexBufferBuilder* float2(const float* data, int elementIdx);
	VertexBufferBuilder* float3(const float* data, int elementIdx);
	VertexBufferBuilder* float4(const float* data, int elementIdx);
	VertexBufferBuilder* floatn(const float* data, int size, int elementIdx);
	VertexBufferBuilder* int1(const int* data, int elementIdx);
	VertexBufferBuilder* int2(const int* data, int elementIdx);
	VertexBufferBuilder* int3(const int* data, int elementIdx);
	VertexBufferBuilder* int4(const int* data, int elementIdx);
	VertexBufferBuilder* intn(const int* data, int size, int elementIdx);
    VertexBufferBuilder* ub4(const unsigned char* data, int elementIdx);
    VertexBufferBuilder* end();

    void Build(Renderer* renderer, VertexBuffer* buffer);
    void Upload(Renderer* renderer, VertexBuffer* buffer);
    void Reset();
	void Grow(size_t count);
	void CalcFaceNormal();

	void SetPosition(size_t vertIndex, float x, float y, float z);
	void SetPosition(size_t vertIndex, const Vec3& position);
	void SetNormal(size_t vertIndex, float x, float y, float z);
	void SetNormal(size_t vertIndex, const Vec3& normal);
	void SetColor(size_t vertIndex, int colorRGBA);
	void SetColor(size_t vertIndex, const Rgba8& color);
	void SetColor(size_t vertIndex, const RgbaF& color);
	void SetColor(size_t vertIndex, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	void SetUV(size_t vertIndex, float u, float v, int compIdx = 0);
	void SetUV(size_t vertIndex, const Vec2& texcoords, int compIdx = 0);

	void* Data();
	size_t Count() const;
	size_t GetBufferSize() const;
	const VertexFormat& GetFormat() const;


private:
	VertexFormat              m_format;
	std::vector<float>        m_data;
	int                       m_stride = 0;
	size_t                    m_count = 0;
	size_t                    m_offset = 0;

	int m_offsetPosition = -1;
	int m_offsetNormal = -1;
	int m_offsetColor = -1;
    int m_offsetUVs[ENGINE_VERTEX_FORMAT_MAX_TEXCOORDS] = { -1 };
	int m_offsets[ENGINE_VERTEX_FORMAT_MAX_ELEMENTS] = { -1 };
};

