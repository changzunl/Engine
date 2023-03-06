#include "VertexFormat.hpp"

#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/RgbaF.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"

#define DEFINE_D3D11_LIBS
#include "ID3D11Internal.hpp"

VertexFormat::VertexFormat()
{
}

void VertexFormat::AddElement(const VertexFormatElement& element)
{
	if (m_elements.size() + 1 >= ENGINE_VERTEX_FORMAT_MAX_ELEMENTS)
		ERROR_AND_DIE("too many texcoord elements");

	int stride = m_stride;
	m_pos[m_elements.size()] = stride;
	m_elements.push_back(element);
	m_stride += VertexFormatElement::GetSize(element.m_format);

	switch (element.m_semantic)
	{
	case VertexFormatElement::Semantic::POSITION:
		ASSERT_OR_DIE(m_posPosition == -1, "position already present");
		m_posPosition = stride;
		break;
	case VertexFormatElement::Semantic::NORMAL:
		ASSERT_OR_DIE(m_posNormal == -1, "normal already present");
		m_posNormal = stride;
		break;
	case VertexFormatElement::Semantic::COLOR:
		ASSERT_OR_DIE(m_posColor == -1, "color already present");
		m_posColor = stride;
		break;
	case VertexFormatElement::Semantic::TEXCOORD:
		for (int idx = 0; idx < ENGINE_VERTEX_FORMAT_MAX_TEXCOORDS; idx++)
		{
			if (m_posUVs[idx] == -1)
			{
				m_posUVs[idx] = stride;
				goto SWITCH_CASE_BREAK;
			}
		}
		ERROR_AND_DIE("too many texcoord elements");
	SWITCH_CASE_BREAK:
		break;
	default:
		break;
	}
}

int VertexFormat::GetVertexStride() const
{
	return m_stride;
}

void VertexFormat::GetInputElements(std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc) const
{
	layoutDesc.reserve(m_elements.size());

	D3D11_INPUT_ELEMENT_DESC desc = { nullptr, 0, DXGI_FORMAT_UNKNOWN, (UINT)m_bufferSlot, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	if (m_instanced)
	{
		desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		desc.InstanceDataStepRate = 1;
	}

	UINT size = 0;
	for (const auto& element : m_elements)
	{
		desc.SemanticName = element.m_name.c_str();
		desc.Format = (DXGI_FORMAT)VertexFormatElement::GetD3DConstant(element.m_format);
		layoutDesc.push_back(desc);
		size += VertexFormatElement::GetSize(element.m_format);
	}
}

VertexFormat VertexFormat::GetDefaultFormat_Vertex_PCU()
{
	VertexFormat format;
	format.AddElement(VertexFormatElement::POSITION3F);
	format.AddElement(VertexFormatElement::COLOR4UB);
	format.AddElement(VertexFormatElement::TEXCOORD2F);

	return format;
}

VertexFormat VertexFormat::GetDefaultFormat_Vertex_PNCU()
{
	VertexFormat format;
	format.AddElement(VertexFormatElement::POSITION3F);
	format.AddElement(VertexFormatElement::NORMAL3F);
	format.AddElement(VertexFormatElement::COLOR4UB);
	format.AddElement(VertexFormatElement::TEXCOORD2F);

	return format;
}

int VertexFormatElement::GetSize(Format format)
{
	constexpr int formatSize[(int)Format::SIZE] = { 4, 4, 4, 4, 8, 12, 16, 12 };
	return formatSize[(int)format];
}

int VertexFormatElement::GetD3DConstant(Format format)
{
	constexpr int formatConstant[(int)Format::SIZE] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT };
	return formatConstant[(int)format];
}

VertexFormatElement VertexFormatElement::POSITION3F("POSITION", Format::FLOAT3, Semantic::POSITION);

VertexFormatElement VertexFormatElement::NORMAL3F("NORMAL", Format::NORM3, Semantic::NORMAL);

VertexFormatElement VertexFormatElement::COLOR4UB("COLOR", Format::UB4_RGBA, Semantic::COLOR);

VertexFormatElement VertexFormatElement::TEXCOORD2F("TEXCOORD", Format::FLOAT2, Semantic::TEXCOORD);

VertexFormatElement::VertexFormatElement(std::string name, Format format, Semantic semantic)
	: m_name(name)
	, m_format(format)
	, m_semantic(semantic)
{
}


void VertexBufferBuilder::Start(const VertexFormat& format, int count)
{
	ASSERT_OR_DIE(format.GetVertexStride() % 4 == 0, "vertex format not aligned");
	m_format = format;
	m_stride = m_format.GetVertexStride() / 4;
	m_data.reserve(m_stride * (size_t)count);
	m_count = 0;
	m_offset = 0;
	m_offsetPosition = format.m_posPosition == -1 ? -1 : (format.m_posPosition / 4);
	m_offsetNormal   = format.m_posNormal   == -1 ? -1 : (format.m_posNormal   / 4);
	m_offsetColor    = format.m_posColor    == -1 ? -1 : (format.m_posColor    / 4);
	for (int i = 0; i < ENGINE_VERTEX_FORMAT_MAX_TEXCOORDS; i++)
		m_offsetUVs[i] = format.m_posUVs[i] == -1 ? -1 : (format.m_posUVs[i] / 4);
	for (int i = 0; i < ENGINE_VERTEX_FORMAT_MAX_ELEMENTS; i++)
		m_offsets[i] = format.m_pos[i]      == -1 ? -1 : (format.m_pos[i]    / 4);
}

VertexBufferBuilder* VertexBufferBuilder::begin()
{
	m_offset = m_stride * m_count;
	m_data.resize(m_stride * ++m_count);
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::pos(const Vec3& position)
{
	if (m_offsetPosition != -1)
	{
		m_data[m_offset + m_offsetPosition + 0] = position.x;
		m_data[m_offset + m_offsetPosition + 1] = position.y;
		m_data[m_offset + m_offsetPosition + 2] = position.z;
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::pos(float x, float y, float z)
{
	if (m_offsetPosition != -1)
	{
		m_data[m_offset + m_offsetPosition + 0] = x;
		m_data[m_offset + m_offsetPosition + 1] = y;
		m_data[m_offset + m_offsetPosition + 2] = z;
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::normal(const Vec3& normal)
{
	if (m_offsetNormal != -1)
	{
		m_data[m_offset + m_offsetNormal + 0] = normal.x;
		m_data[m_offset + m_offsetNormal + 1] = normal.y;
		m_data[m_offset + m_offsetNormal + 2] = normal.z;
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::normal(float x, float y, float z)
{
	if (m_offsetNormal != -1)
	{
		m_data[m_offset + m_offsetNormal + 0] = x;
		m_data[m_offset + m_offsetNormal + 1] = y;
		m_data[m_offset + m_offsetNormal + 2] = z;
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::color(const Rgba8& color)
{
	if (m_offsetColor != -1)
	{
		float* data = (float*)&color;
		m_data[m_offset + m_offsetColor] = data[0];
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::color(const RgbaF& colorF)
{
	if (m_offsetColor != -1)
	{
		Rgba8 color = colorF.GetAsRgba8();
		float* data = (float*)&color; // BUG: potential memory alignment bug, find better solution
		m_data[m_offset + m_offsetColor] = data[0];
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::color(int colorRGBA)
{
	if (m_offsetColor != -1)
	{
		Rgba8 color = Rgba8((unsigned char)(colorRGBA >> 24), (unsigned char)(colorRGBA >> 16), (unsigned char)(colorRGBA >> 8), (unsigned char)(colorRGBA));
		float* data = (float*)&color;
		m_data[m_offset + m_offsetColor] = data[0];
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::color(unsigned char r, unsigned char g, unsigned char b, unsigned char a /*= 255*/)
{
	return color(Rgba8(r, g, b, a));
}

VertexBufferBuilder* VertexBufferBuilder::tex(const Vec2& texcoords, int index /*= 0*/)
{
	int offsetUV = m_offsetUVs[index];
	if (offsetUV != -1)
	{
		m_data[m_offset + offsetUV + 0] = texcoords.x;
		m_data[m_offset + offsetUV + 1] = texcoords.y;
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::tex(float u, float v, int index /*= 0*/)
{
	int offsetUV = m_offsetUVs[index];
	if (offsetUV != -1)
	{
		m_data[m_offset + offsetUV + 0] = u;
		m_data[m_offset + offsetUV + 1] = v;
	}
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::float1(const float* data, int elementIdx)
{
	constexpr int n = 1;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
		m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::float2(const float* data, int elementIdx)
{
	constexpr int n = 2;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::float3(const float* data, int elementIdx)
{
	constexpr int n = 3;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::float4(const float* data, int elementIdx)
{
	constexpr int n = 4;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::floatn(const float* data, int size, int elementIdx)
{
	int offset = m_offsets[elementIdx];
	if (offset != -1)
		memcpy(&m_data[m_offset + offset], data, size * sizeof(float));
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::int1(const int* _data, int elementIdx)
{
	constexpr int n = 1;
	const float* data = (const float*)_data;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::int2(const int* _data, int elementIdx)
{
	constexpr int n = 2;
	const float* data = (const float*)_data;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::int3(const int* _data, int elementIdx)
{
	constexpr int n = 3;
	const float* data = (const float*)_data;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::int4(const int* _data, int elementIdx)
{
	constexpr int n = 4;
	const float* data = (const float*)_data;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
		for (int i = 0; i < n; i++)
			m_data[m_offset + offset + i] = data[i];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::intn(const int* _data, int size, int elementIdx)
{
	const float* data = (const float*)_data;
	int offset = m_offsets[elementIdx];
	if (offset != -1)
		memcpy(&m_data[m_offset + offset], data, size * sizeof(int));
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::ub4(const unsigned char* _data, int elementIdx)
{
	const float* data = (const float*)_data;

	int offset = m_offsets[elementIdx];
	if (offset != -1)
			m_data[m_offset + offset] = data[0];
	return this;
}

VertexBufferBuilder* VertexBufferBuilder::end()
{
	m_offset = 0;
	return this;
}

void VertexBufferBuilder::CalcFaceNormal()
{
	if (m_offsetNormal == -1 || m_count % 3 != 0)
		return;

	size_t offset0 = (m_count - 3) * m_stride;
	size_t offset1 = (m_count - 2) * m_stride;
	size_t offset2 = (m_count - 1) * m_stride;
	Vec3 point0;
	Vec3 point1;
	Vec3 point2;
	point0.x = m_data[offset0 + m_offsetPosition + 0];
	point0.y = m_data[offset0 + m_offsetPosition + 1];
	point0.z = m_data[offset0 + m_offsetPosition + 2];
	point1.x = m_data[offset1 + m_offsetPosition + 0];
	point1.y = m_data[offset1 + m_offsetPosition + 1];
	point1.z = m_data[offset1 + m_offsetPosition + 2];
	point2.x = m_data[offset2 + m_offsetPosition + 0];
	point2.y = m_data[offset2 + m_offsetPosition + 1];
	point2.z = m_data[offset2 + m_offsetPosition + 2];

	Vec3 normal = (point1 - point0).Cross(point2 - point1).GetNormalized();

	m_data[offset0 + m_offsetNormal + 0] = normal.x;
	m_data[offset0 + m_offsetNormal + 1] = normal.y;
	m_data[offset0 + m_offsetNormal + 2] = normal.z;
	m_data[offset1 + m_offsetNormal + 0] = normal.x;
	m_data[offset1 + m_offsetNormal + 1] = normal.y;
	m_data[offset1 + m_offsetNormal + 2] = normal.z;
	m_data[offset2 + m_offsetNormal + 0] = normal.x;
	m_data[offset2 + m_offsetNormal + 1] = normal.y;
	m_data[offset2 + m_offsetNormal + 2] = normal.z;
}

size_t VertexBufferBuilder::Count() const
{
	return m_count;
}

size_t VertexBufferBuilder::GetBufferSize() const
{
	return m_data.size() * sizeof(float);
}

const VertexFormat& VertexBufferBuilder::GetFormat() const
{
	return m_format;
}

void VertexBufferBuilder::SetPosition(size_t vertIndex, float x, float y, float z)
{
	if (m_offsetPosition != -1)
	{
		size_t offset = vertIndex * m_stride;
		m_data[offset + m_offsetPosition + 0] = x;
		m_data[offset + m_offsetPosition + 1] = y;
		m_data[offset + m_offsetPosition + 2] = z;
	}
}

void VertexBufferBuilder::SetPosition(size_t vertIndex, const Vec3& position)
{
	if (m_offsetPosition != -1)
	{
		size_t offset = vertIndex * m_stride;
		m_data[offset + m_offsetPosition + 0] = position.x;
		m_data[offset + m_offsetPosition + 1] = position.y;
		m_data[offset + m_offsetPosition + 2] = position.z;
	}
}

void VertexBufferBuilder::SetNormal(size_t vertIndex, float x, float y, float z)
{
	if (m_offsetNormal != -1)
	{
		size_t offset = vertIndex * m_stride;
		m_data[offset + m_offsetNormal + 0] = x;
		m_data[offset + m_offsetNormal + 1] = y;
		m_data[offset + m_offsetNormal + 2] = z;
	}
}

void VertexBufferBuilder::SetNormal(size_t vertIndex, const Vec3& normal)
{
	if (m_offsetNormal != -1)
	{
		size_t offset = vertIndex * m_stride;
		m_data[offset + m_offsetNormal + 0] = normal.x;
		m_data[offset + m_offsetNormal + 1] = normal.y;
		m_data[offset + m_offsetNormal + 2] = normal.z;
	}
}

void VertexBufferBuilder::SetColor(size_t vertIndex, int colorRGBA)
{
	if (m_offsetColor != -1)
	{
		size_t offset = vertIndex * m_stride;
		float* data = (float*)&colorRGBA;
		m_data[offset + m_offsetColor] = data[0];
	}
}

void VertexBufferBuilder::SetColor(size_t vertIndex, const Rgba8& color)
{
	if (m_offsetColor != -1)
	{
		size_t offset = vertIndex * m_stride;
		float* data = (float*)&color;
		m_data[offset + m_offsetColor] = data[0];
	}
}

void VertexBufferBuilder::SetColor(size_t vertIndex, const RgbaF& color)
{
	SetColor(vertIndex, color.GetAsRgba8());
}

void VertexBufferBuilder::SetColor(size_t vertIndex, unsigned char r, unsigned char g, unsigned char b, unsigned char a /*= 255*/)
{
	SetColor(vertIndex, Rgba8(r, g, b, a));
}

void VertexBufferBuilder::SetUV(size_t vertIndex, float u, float v, int index /*= 0*/)
{
	int offsetUV = m_offsetUVs[index];
	if (offsetUV != -1)
	{
		size_t offset = vertIndex * m_stride;
		m_data[offset + offsetUV + 0] = u;
		m_data[offset + offsetUV + 1] = v;
	}
}

void VertexBufferBuilder::SetUV(size_t vertIndex, const Vec2& texcoords, int index /*= 0*/)
{
	int offsetUV = m_offsetUVs[index];
	if (offsetUV != -1)
	{
		size_t offset = vertIndex * m_stride;
		m_data[offset + offsetUV + 0] = texcoords.x;
		m_data[offset + offsetUV + 1] = texcoords.y;
	}
}

void VertexBufferBuilder::Build(Renderer* renderer, VertexBuffer* buffer)
{
	Upload(renderer, buffer);
	Reset();
}

void VertexBufferBuilder::Upload(Renderer* renderer, VertexBuffer* buffer)
{
	renderer->CopyCPUToGPU(m_data.data(), m_data.size() * sizeof(float), buffer);
}

void* VertexBufferBuilder::Data()
{
	return m_data.data();
}

void VertexBufferBuilder::Reset()
{
	m_format = VertexFormat();
	m_stride = 0;
	m_data.clear();
	m_count = 0;
	m_offset = 0;
	m_offsetPosition = -1;
	m_offsetNormal = -1;
	m_offsetColor = -1;
	for (int i = 0; i < ENGINE_VERTEX_FORMAT_MAX_TEXCOORDS; i++)
		m_offsetUVs[i] = -1;
	for (int i = 0; i < ENGINE_VERTEX_FORMAT_MAX_ELEMENTS; i++)
		m_offsets[i] = -1;
}

void VertexBufferBuilder::Clear()
{
    m_data.clear();
    m_count = 0;
    m_offset = 0;
    std::vector<float> zero_vector;
    m_data.swap(zero_vector);
}

void VertexBufferBuilder::Grow(size_t count)
{
	m_data.reserve(m_data.size() + m_stride * count);
}

