#include "Engine/Animation/Skeleton.hpp"

#include "Engine/Core/ByteBuffer.hpp"

Skeleton::Skeleton()
	: m_defaultPose(this)
{
}

Pose& Skeleton::GetPose()
{
	return m_defaultPose;
}

const Pose& Skeleton::GetPose() const
{
	return m_defaultPose;
}

BoneId Skeleton::GetRoot() const
{
	return m_root;
}

void Skeleton::SetRoot(BoneId root)
{
	m_root = root;

	m_defaultPose.Initialize();
}

void Skeleton::SetBones(int num)
{
	m_bones.resize(num);
	m_defaultPose.m_boneLocalPose.resize(size());
	m_defaultPose.m_boneCompPose.resize(size());
}

size_t Skeleton::size() const
{
	return m_bones.size();
}

Bone* Skeleton::begin()
{
	return &m_bones.data()[0];
}

Bone* Skeleton::end()
{
	return &m_bones.data()[m_bones.size()];
}

const Bone* Skeleton::begin() const
{
	return &m_bones.data()[0];
}

const Bone* Skeleton::end() const
{
	return &m_bones.data()[m_bones.size()];
}

void Skeleton::ReadBytes(ByteBuffer* byteBuf)
{
	char IDENTIFIER[4];
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "SKEL"
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "DATA"
	BoneId invalidId;
	byteBuf->Read(invalidId);
	byteBuf->Read(m_root);
	ByteUtils::ReadObjects(byteBuf, m_bones);
	m_defaultPose.ReadBytes(byteBuf);
}

void Skeleton::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(4, "SKEL");
	byteBuf->Write(4, "DATA");
	byteBuf->Write(INVALID_BONE_ID);
	byteBuf->Write(m_root);
	ByteUtils::WriteObjects(byteBuf, m_bones);
	m_defaultPose.WriteBytes(byteBuf);
}

void Bone::ReadBytes(ByteBuffer* byteBuf)
{
	char IDENTIFIER[4];
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "BONE"
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "DATA"
	ByteUtils::ReadString(byteBuf, m_name);
	byteBuf->Read(m_id);
	byteBuf->Read(m_parentId);
	ByteUtils::ReadArray(byteBuf, m_children);
	byteBuf->ReadAlignment();

	byteBuf->Read(m_transform);
	byteBuf->Read(m_offset);
}

void Bone::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(4, "BONE");
	byteBuf->Write(4, "DATA");
	ByteUtils::WriteString(byteBuf, m_name);
	byteBuf->Write(m_id);
	byteBuf->Write(m_parentId);
	ByteUtils::WriteArray(byteBuf, m_children);
	byteBuf->WriteAlignment();

	byteBuf->Write(m_transform);
	byteBuf->Write(m_offset);
}

Pose::Pose(const Skeleton* skeleton)
	: m_skeleton(skeleton)
{
}

Pose::Pose(const Pose& copyFrom)
	: m_skeleton(copyFrom.m_skeleton)
	, m_bakedPose(copyFrom.m_bakedPose)
	, m_boneLocalPose(copyFrom.m_boneLocalPose)
	, m_boneCompPose(copyFrom.m_boneCompPose)
{
}

void Pose::operator=(const Pose& copyFrom)
{
	if (m_skeleton == copyFrom.m_skeleton)
	{
		m_bakedPose = copyFrom.m_bakedPose;
		m_boneLocalPose = copyFrom.m_boneLocalPose;
		m_boneCompPose = copyFrom.m_boneCompPose;
	}
}

void Pose::Initialize()
{
	m_boneLocalPose.resize(m_skeleton->size());
	m_boneCompPose.resize(m_skeleton->size());
	Reset();
}

const Skeleton& Pose::GetSkeleton() const
{
	return *m_skeleton;
}

void Pose::Reset()
{
	for (auto& bone : GetSkeleton())
		m_boneLocalPose[bone.m_id] = TransformQuat::DecomposeAffineMatrix(bone.m_transform);
}

void Pose::Bake()
{
	m_bakedPose = SkeletonConstants();
	BakeBone(GetSkeleton().GetRoot(), Mat4x4::IDENTITY);
}

void Pose::BakeFromComp()
{
	m_bakedPose = SkeletonConstants();
	for (auto& bone : GetSkeleton())
		m_bakedPose[bone.m_id] = m_boneCompPose[bone.m_id].GetMatrix() * bone.m_offset;
}

void Pose::BakeLocalToComp()
{
	BakeBoneLocalToComp(GetSkeleton().GetRoot(), Mat4x4::IDENTITY);
}

void Pose::BakeCompToLocal()
{
	BakeBoneCompToLocal(GetSkeleton().GetRoot(), Mat4x4::IDENTITY);
}

void Pose::BakeBone(BoneId boneId, const Mat4x4& parentM)
{
	const Bone& bone = *m_skeleton->FindBone(boneId);

	Mat4x4 boneM = m_boneLocalPose[bone.m_id].GetMatrix();

	Mat4x4 globalM = parentM * boneM;

	m_bakedPose[boneId] = globalM * bone.m_offset;

	for (BoneId child : bone.m_children)
		BakeBone(child, globalM);
}

void Pose::BakeBoneCompToLocal(BoneId boneId, const Mat4x4& parentGlobalM)
{
	const Bone& bone = *m_skeleton->FindBone(boneId);

	Mat4x4 boneGlobalM = m_boneCompPose[boneId].GetMatrix();

	// AB = C then B = A-1C
	Mat4x4 boneLocalM = parentGlobalM.GetOrthonormalInverse() * boneGlobalM;

	m_boneLocalPose[boneId] = TransformQuat::DecomposeAffineMatrix(boneLocalM);

	for (BoneId child : bone.m_children)
		BakeBoneCompToLocal(child, boneGlobalM);
}

void Pose::BakeBoneLocalToComp(BoneId boneId, const Mat4x4& parentM)
{
	const Bone& bone = *m_skeleton->FindBone(boneId);

	Mat4x4 boneLocalM = m_boneLocalPose[bone.m_id].GetMatrix();

	Mat4x4 boneGlobalM = parentM * boneLocalM;

	m_boneCompPose[boneId] = TransformQuat::DecomposeAffineMatrix(boneGlobalM);

	for (BoneId child : bone.m_children)
		BakeBoneLocalToComp(child, boneGlobalM);
}

void Pose::ReadBytes(ByteBuffer* byteBuf)
{
	m_boneLocalPose.resize(m_skeleton->size());
	m_boneCompPose.resize(m_skeleton->size());

	byteBuf->Read(m_bakedPose);
	byteBuf->Read(m_skeleton->size(), m_boneLocalPose.data());
	byteBuf->Read(m_skeleton->size(), m_boneCompPose.data());
}

void Pose::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(m_bakedPose);
	byteBuf->Write(m_skeleton->size(), m_boneLocalPose.data());
	byteBuf->Write(m_skeleton->size(), m_boneCompPose.data());
}

