#pragma once

#include "Engine/Animation/Mesh.hpp"
#include "Engine/Animation/Skeleton.hpp"

#define ENGINE_SKEL_MAX_BONE_WEIGHTS 4

class ByteBuffer;

struct BoneIndices
{
	unsigned char indices[ENGINE_SKEL_MAX_BONE_WEIGHTS];
};


struct BoneWeights
{
	float weights[ENGINE_SKEL_MAX_BONE_WEIGHTS];
};


class SkeletalMesh : public Mesh
{
public:
	inline SkeletalMesh();
	inline SkeletalMesh(const char* name);
	inline ~SkeletalMesh();

	// Serialization
	virtual void ReadBytes(ByteBuffer* byteBuf) override;
	virtual void WriteBytes(ByteBuffer* byteBuf) const override;

public:
	Skeleton                    m_skeleton;
	std::vector<BoneIndices>    m_boneIndices;
	std::vector<BoneWeights>    m_boneWeights;
};


/* =============================================================*/
/* ================   INLINE IMPLEMENTATION   ==================*/
/* =============================================================*/


SkeletalMesh::SkeletalMesh()
{
}


SkeletalMesh::SkeletalMesh(const char* name) 
	: Mesh(name) 
{
}


SkeletalMesh:: ~SkeletalMesh() 
{
}

