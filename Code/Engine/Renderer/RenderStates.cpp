#include "Engine/Renderer/RenderStates.hpp"


void RasterizerState::SetCullMode(CullMode cullMode)
{
	if (m_cullMode != cullMode)
	{
		m_cullMode = cullMode;
		m_stateChanged = true;
	}
}

void RasterizerState::SetFillMode(FillMode fillMode)
{
	if (m_fillMode != fillMode)
	{
		m_fillMode = fillMode;
		m_stateChanged = true;
	}
}

void RasterizerState::SetWindingOrder(WindingOrder windingOrder)
{
	if (m_windingOrder != windingOrder)
	{
		m_windingOrder = windingOrder;
		m_stateChanged = true;
	}
}

void RasterizerState::SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder)
{
	if (m_cullMode != cullMode || m_fillMode != fillMode || m_windingOrder != windingOrder)
	{
		m_cullMode = cullMode;
		m_fillMode = fillMode;
		m_windingOrder = windingOrder;
		m_stateChanged = true;
	}
}

void DepthStencilState::SetDepthTest(DepthTest depthTest)
{
	if (m_depthTest != depthTest)
	{
		m_depthTest = depthTest;
		m_stateChanged = true;
	}
}

void DepthStencilState::SetDepthMask(bool writeDepth)
{
	if (m_depthMask != writeDepth)
	{
		m_depthMask = writeDepth;
		m_stateChanged = true;
	}
}

void DepthStencilState::SetDepthStencilState(DepthTest depthTest, bool writeDepth)
{
	if (m_depthTest != depthTest || m_depthMask != writeDepth)
	{
		m_depthTest = depthTest;
		m_depthMask = writeDepth;
		m_stateChanged = true;
	}
}

void DepthStencilState::SetStencilTest(bool enabled)
{
    if (m_stencilTest != enabled)
    {
		m_stencilTest = enabled;
        m_stateChanged = true;
    }
}

void DepthStencilState::SetStencilWrite(bool enabled)
{
    if (m_stencilWrite != enabled)
    {
        m_stencilWrite = enabled;
        m_stateChanged = true;
    }
}

void BlendState::SetBlendMode(BlendMode mode)
{
	if (m_blendMode != mode)
	{
		m_blendMode = mode;
		m_stateChanged = true;
	}
}

void SamplerState::SetSamplerMode(SamplerMode mode)
{
	if (m_samplerMode != mode)
	{
		m_samplerMode = mode;
		m_stateChanged = true;
	}
}

