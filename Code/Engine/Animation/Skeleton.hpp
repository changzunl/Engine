#pragma once

#include "Engine/Animation/Quaternion.hpp"

#include <string>
#include <vector>

#define ENGINE_SKEL_MAX_BONES 128

typedef unsigned char BoneId;
constexpr BoneId INVALID_BONE_ID = 255;

class ByteBuffer;
class Skeleton;


// ========================================================================================
// ========================================================================================
struct SkeletonConstants
{
public:
	inline const void*      GetBuffer() const;

	inline Mat4x4&          operator[](size_t pos);
	inline const Mat4x4&    operator[](size_t pos) const;

private:
	Mat4x4                  m_matrices[ENGINE_SKEL_MAX_BONES];
};


// ========================================================================================
// ========================================================================================
struct Pose
{
	friend class Skeleton;

private:
	Pose(const Skeleton* skeleton);

	void Initialize();

public:
	Pose(const Pose& copyFrom);
	void operator=(const Pose& copyFrom);

	const Skeleton& GetSkeleton() const;

	void Reset();
	void Bake();
	void BakeFromComp();
	void BakeLocalToComp();
	void BakeCompToLocal();

	void BakeBone(BoneId boneId, const Mat4x4& parentM);
	void BakeBoneCompToLocal(BoneId boneId, const Mat4x4& parentM);
	void BakeBoneLocalToComp(BoneId boneId, const Mat4x4& parentM);

	// Serialization
	void ReadBytes(ByteBuffer* byteBuf);
	void WriteBytes(ByteBuffer* byteBuf) const;

public:
	const Skeleton* const m_skeleton;
	SkeletonConstants m_bakedPose;
	std::vector<TransformQuat> m_boneLocalPose;
	std::vector<TransformQuat> m_boneCompPose;
};


// ========================================================================================
// ========================================================================================
struct Bone
{
public:
	std::string m_name;
	BoneId m_id = INVALID_BONE_ID;
	BoneId m_parentId = INVALID_BONE_ID;
	Mat4x4 m_transform; // from aiNode
	Mat4x4 m_offset; // from aiBone
	std::vector<BoneId> m_children;

	// Serialization
	void ReadBytes(ByteBuffer* byteBuf);
	void WriteBytes(ByteBuffer* byteBuf) const;
};


// ========================================================================================
// ========================================================================================
class Skeleton
{
public:
	Skeleton();

	Pose&                   GetPose();
	const Pose&             GetPose() const;

	BoneId                  GetRoot() const;
	void                    SetRoot(BoneId root);
	void                    SetBones(int num);
	inline BoneId           FindBone(const char* name) const;
	inline Bone*            FindBone(BoneId boneId);
	inline const Bone*      FindBone(BoneId boneId) const;

	size_t                  size() const;
	Bone*                   begin();
	Bone*                   end();
	const Bone*             begin() const;
	const Bone*             end() const;

	// Serialization
	void                    ReadBytes(ByteBuffer* byteBuf);
	void                    WriteBytes(ByteBuffer* byteBuf) const;

private:
	BoneId                  m_root = INVALID_BONE_ID;
	std::vector<Bone>       m_bones;
	Pose                    m_defaultPose;
};


// ==============================================================================================
// =================================   INLINE FUNCTIONS   =======================================
// ==============================================================================================
BoneId Skeleton::FindBone(const char* name) const
{
	for (auto& bone : m_bones)
		if (bone.m_name == name)
			return bone.m_id;
	return INVALID_BONE_ID;
}


// ========================================================================================
// ========================================================================================
Bone* Skeleton::FindBone(BoneId boneId)
{
	if (boneId >= m_bones.size())
		return nullptr;
	return &m_bones[boneId];
}


// ========================================================================================
// ========================================================================================
const Bone* Skeleton::FindBone(BoneId boneId) const
{
	if (boneId >= m_bones.size())
		return nullptr;
	return &m_bones[boneId];
}


// ========================================================================================
// ========================================================================================
const void* SkeletonConstants::GetBuffer() const
{
	return &m_matrices[0];
}


// ========================================================================================
// ========================================================================================
Mat4x4& SkeletonConstants::operator[](size_t pos)
{
	return m_matrices[pos];
}


// ========================================================================================
// ========================================================================================
const Mat4x4& SkeletonConstants::operator[](size_t pos) const
{
	return m_matrices[pos];
}

