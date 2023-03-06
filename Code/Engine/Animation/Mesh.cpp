#include "Mesh.hpp"

#include "Engine/Core/ByteBuffer.hpp"

void Mesh::ReadBytes(ByteBuffer* buffer)
{
	ByteUtils::ReadString(buffer, m_name);

	int maxUvChannels;
	buffer->Read(maxUvChannels);
	buffer->Read(m_transformation);

	ByteUtils::ReadArray(buffer, m_vertices);
	ByteUtils::ReadArray(buffer, m_normals);
	for (int chn = 0; chn < ENGINE_MESH_MAX_UVCHANNELS; chn++)
		m_uvs[chn].clear();
	for (int chn = 0; chn < maxUvChannels && chn < ENGINE_MESH_MAX_UVCHANNELS; chn++)
		ByteUtils::ReadArray(buffer, m_uvs[chn]);
	ByteUtils::ReadArray(buffer, m_indices);

// 	// Endianness serialization
// 	ByteUtils::ReadString_Net(buffer, m_name);
// 
// 	int maxUvChannels;
// 	buffer->Read(maxUvChannels);
// 	buffer->Read(m_transformation);
// 
// 	if (!ByteUtils::IsPlatformBigEndian())
// 	{
// 		ByteUtils::ReverseBytes(maxUvChannels);
// 		ByteUtils::ReverseBytes32((BYTE*)m_transformation.data());
// 	}
// 
// 	ByteUtils::ReadArray_Net(buffer, m_vertices, DataType::FLOAT);
// 	ByteUtils::ReadArray_Net(buffer, m_normals, DataType::FLOAT);
// 	for (int chn = 0; chn < ENGINE_MESH_MAX_UVCHANNELS; chn++)
// 		m_uvs[chn].clear();
// 	for (int chn = 0; chn < maxUvChannels && chn < ENGINE_MESH_MAX_UVCHANNELS; chn++)
// 		ByteUtils::ReadArray_Net(buffer, m_uvs[chn], DataType::FLOAT);
// 	ByteUtils::ReadArray_Net(buffer, m_indices, DataType::INT);
}


void Mesh::WriteBytes(ByteBuffer* buffer) const
{
	ByteUtils::WriteString(buffer, m_name);

	buffer->Write((int)ENGINE_MESH_MAX_UVCHANNELS);
	buffer->Write(m_transformation);

	ByteUtils::WriteArray(buffer, m_vertices);
	ByteUtils::WriteArray(buffer, m_normals);
	for (int chn = 0; chn < ENGINE_MESH_MAX_UVCHANNELS; chn++)
		ByteUtils::WriteArray(buffer, m_uvs[chn]);
	ByteUtils::WriteArray(buffer, m_indices);

// 	// Endianness serialization
// 	ByteUtils::WriteString_Net(buffer, m_name);
// 
// 	size_t writeIdx = buffer->m_writeIdx;
// 	buffer->Write((int)ENGINE_MESH_MAX_UVCHANNELS);
// 	buffer->Write(m_transformation);
// 
// 	if (!ByteUtils::IsPlatformBigEndian())
// 	{
// 		for (size_t idx = writeIdx; idx < buffer->m_writeIdx; idx += 4)
// 			ByteUtils::ReverseBytes32(&buffer->data()[idx]);
// 	}
// 
// 	ByteUtils::WriteArray_Net(buffer, m_vertices, DataType::FLOAT);
// 	ByteUtils::WriteArray_Net(buffer, m_normals, DataType::FLOAT);
// 	for (int chn = 0; chn < ENGINE_MESH_MAX_UVCHANNELS; chn++)
// 		ByteUtils::WriteArray_Net(buffer, m_uvs[chn], DataType::FLOAT);
// 	ByteUtils::WriteArray_Net(buffer, m_indices, DataType::INT);
}

void Mesh::TransformBasis(const Vec3& i, const Vec3& j, const Vec3& k)
{
	if (i == Vec3(1, 0, 0) && j == Vec3(0, 1, 0) && k == Vec3(0, 0, 1)) // ignore identity transform
		return;

	for (Vec3& vert : m_vertices)
	{
		Vec3 result = {};
		result.x = i.x * vert.x + j.x * vert.y + k.x * vert.z;
		result.y = i.y * vert.x + j.y * vert.y + k.y * vert.z;
		result.z = i.z * vert.x + j.z * vert.y + k.z * vert.z;

		vert = result;
	}

	for (Vec3& vert : m_normals)
	{
		Vec3 result = {};
		result.x = i.x * vert.x + j.x * vert.y + k.x * vert.z;
		result.y = i.y * vert.x + j.y * vert.y + k.y * vert.z;
		result.z = i.z * vert.x + j.z * vert.y + k.z * vert.z;

		vert = result;
	}
}

void Mesh::ScaleMesh(float scale)
{
	if (scale == 1.f) // ignore identity scale
		return;

	for (Vec3& vert : m_vertices)
	{
		vert *= scale;
	}
}

void Mesh::FlipUV()
{
	for (auto& uvs : m_uvs)
	{
		for (Vec2& uv : uvs)
		{
			uv.y = 1 - uv.y;
		}
	}
}

void Mesh::ReverseWindingOrder()
{
	for (size_t i = 0; i < m_indices.size(); i += 3)
	{
		std::swap(m_indices[i + 1], m_indices[i + 2]);
	}
}

void StaticMesh::ReadBytes(ByteBuffer* byteBuf)
{
	char IDENTIFIER[4];
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "STAT"
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "MESH"
	Mesh::ReadBytes(byteBuf);
}

void StaticMesh::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(4, "STAT");
	byteBuf->Write(4, "MESH");
	Mesh::WriteBytes(byteBuf);
}
