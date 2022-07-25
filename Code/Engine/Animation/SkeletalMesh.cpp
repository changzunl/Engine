#include "Engine/Animation/SkeletalMesh.hpp"

#include "Engine/Core/ByteBuffer.hpp"


void SkeletalMesh::ReadBytes(ByteBuffer* byteBuf)
{
	char IDENTIFIER[4];
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "SKEL"
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "MESH"
	Mesh::ReadBytes(byteBuf);

	ByteUtils::ReadArray(byteBuf, m_boneIndices);
	ByteUtils::ReadArray(byteBuf, m_boneWeights);

	m_skeleton.ReadBytes(byteBuf);
}

void SkeletalMesh::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(4, "SKEL");
	byteBuf->Write(4, "MESH");
	Mesh::WriteBytes(byteBuf);

	ByteUtils::WriteArray(byteBuf, m_boneIndices);
	ByteUtils::WriteArray(byteBuf, m_boneWeights);

	m_skeleton.WriteBytes(byteBuf);
}

