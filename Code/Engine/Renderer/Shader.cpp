#include "Shader.hpp"

#include "Renderer.hpp"

#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

Shader::Shader(const ShaderConfig& config)
	: m_config(config)
{

}

Shader::~Shader()
{
	DX_SAFE_RELEASE(m_pixelShader);
	DX_SAFE_RELEASE(m_inputLayout);
	DX_SAFE_RELEASE(m_vertexShader);
}

const std::string& Shader::GetName() const
{
	return m_config.m_name;
}

int Shader::GetInputFormatSize() const
{
	return (int)m_config.m_formats.size();
}

const VertexFormat& Shader::GetInputFormat(int index) const
{
	return m_config.m_formats[index];
}

