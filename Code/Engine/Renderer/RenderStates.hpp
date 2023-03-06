#pragma once

class Texture;

struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11BlendState;
struct ID3D11SamplerState;

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE,
	SIZE,
};

enum class CullMode
{
	NONE,
	FRONT,
	BACK,
};

enum class FillMode
{
	SOLID,
	WIREFRAME,
};

enum class WindingOrder
{
	CLOCKWISE,
	COUNTERCLOCKWISE,
};

enum class DepthTest
{
	ALWAYS,
	NEVER,
	EQUAL,
	NOTEQUAL,
	LESS,
	LESSEQUAL,
	GREATER,
	GREATEREQUAL,
};

enum class SamplerMode
{
	POINTCLAMP,
	POINTWRAP,
	BILINEARCLAMP,
	BILINEARWRAP,
	SIZE,
};

int GetD3DConstant(CullMode cullMode);
int GetD3DConstant(FillMode fillMode);
int GetD3DConstant(WindingOrder windingOrder);
int GetD3DConstant(DepthTest depthTest);

struct RasterizerState
{
	friend class Renderer;

public:
	void SetCullMode(CullMode cullMode);
	void SetFillMode(FillMode fillMode);
	void SetWindingOrder(WindingOrder windingOrder);
	void SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder);

private:
	void Release();

private:
	CullMode               m_cullMode = CullMode::NONE;
	FillMode               m_fillMode = FillMode::SOLID;
	WindingOrder           m_windingOrder = WindingOrder::COUNTERCLOCKWISE;

	bool                   m_stateChanged = true;
	ID3D11RasterizerState* m_d3dRasterizerState = nullptr;
};

struct DepthStencilState
{
	friend class Renderer;

public:
	void SetDepthTest(DepthTest depthTest);
	void SetDepthMask(bool writeDepth);
	void SetDepthStencilState(DepthTest depthTest, bool writeDepth);
	void SetStencilTest(bool enabled);
	void SetStencilWrite(bool enabled);

private:
	void Release();

private:
	DepthTest                m_depthTest = DepthTest::ALWAYS;
	bool                     m_depthMask = false;
	bool                     m_stencilTest = false;
	bool                     m_stencilWrite = false;

	bool                     m_stateChanged = true;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView*  m_depthStencilView = nullptr;
	Texture*                 m_depthStencilTexture = nullptr;
};

struct BlendState
{
	friend class Renderer;

public:
	void SetBlendMode(BlendMode mode);

private:
	void Release();

private:
	BlendMode         m_blendMode                         = BlendMode::OPAQUE;

	bool              m_stateChanged = true;
	ID3D11BlendState* m_blendStates[(int)BlendMode::SIZE] = {};
};

struct SamplerState
{
	friend class Renderer;

public:
	void SetSamplerMode(SamplerMode mode);

private:
	void Release();

private:
	SamplerMode         m_samplerMode                           = SamplerMode::POINTWRAP;

	bool                m_stateChanged                          = true;
	ID3D11SamplerState* m_samplerStates[(int)SamplerMode::SIZE] = {};
};

