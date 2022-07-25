#include "Engine/Animation/Animation.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Engine/Core/ByteBuffer.hpp"

#include <deque>

AnimationCurve::AnimationCurve(int numPosKeys, int numRotKeys, int numScaleKeys)
{
	Initialize(numPosKeys, numRotKeys, numScaleKeys);
}

AnimationCurve::AnimationCurve(const AnimationCurve& copyFrom)
	: AnimationCurve(copyFrom.m_numPosKeys, copyFrom.m_numRotKeys, copyFrom.m_numScaleKeys)
{
	memcpy(m_posTimes,     copyFrom.m_posTimes,     sizeof(float) * m_numPosKeys);
	memcpy(m_rotTimes,     copyFrom.m_rotTimes,     sizeof(float) * m_numRotKeys);
	memcpy(m_scalingTimes, copyFrom.m_scalingTimes, sizeof(float) * m_numScaleKeys);

	memcpy(m_positions, copyFrom.m_positions, sizeof(Vec3) * m_numPosKeys);
	memcpy(m_rotations, copyFrom.m_rotations, sizeof(Quaternion) * m_numRotKeys);
	memcpy(m_scalings,  copyFrom.m_scalings,  sizeof(Vec3) * m_numScaleKeys);
}

AnimationCurve::AnimationCurve()
{
}

AnimationCurve::~AnimationCurve()
{
	delete[] m_posTimes;
	delete[] m_rotTimes;
	delete[] m_scalingTimes;
	delete[] m_positions;
	delete[] m_rotations;
	delete[] m_scalings;
}

void AnimationCurve::Initialize(int numPosKeys, int numRotKeys, int numScaleKeys)
{
	delete[] m_posTimes;
	delete[] m_rotTimes;
	delete[] m_scalingTimes;
	delete[] m_positions;
	delete[] m_rotations;
	delete[] m_scalings;

	m_numPosKeys = numPosKeys;
	m_numRotKeys = numRotKeys;
	m_numScaleKeys = numScaleKeys;
	m_posTimes = new float[numPosKeys] {};
	m_rotTimes = new float[numRotKeys] {};
	m_scalingTimes = new float[numScaleKeys] {};
	m_positions = new Vec3[numPosKeys] {};
	m_rotations = new Quaternion[numRotKeys] {};
	m_scalings = new Vec3[numScaleKeys] {};
}

void AnimationCurve::ResolveAt(Vec3& pos, Quaternion& rot, Vec3& scale, float tick) const
{
	if (m_numPosKeys != 0)
	{
		if (tick < m_posTimes[0])
		{
			pos = m_positions[0];
		}
		else if (tick >= m_posTimes[m_numPosKeys - 1])
		{
			pos = m_positions[m_numPosKeys - 1];
		}
		else
		{
			bool success = false;
			for (int i = 0; i < m_numPosKeys - 1; i++)
			{
				float time1 = m_posTimes[i];
				float time2 = m_posTimes[i + 1];
				if (tick >= time1 && tick < time2)
				{
					pos = Lerp(m_positions[i], m_positions[i + 1], GetFractionWithin(tick, time1, time2));
					success = true;
					break;
				}
			}
			if (!success)
				ERROR_RECOVERABLE("Unexpected position timeline!");
		}
	}

	if (m_numRotKeys != 0)
	{
		if (tick < m_rotTimes[0])
		{
			rot = m_rotations[0];
		}
		else if (tick >= m_rotTimes[m_numRotKeys - 1])
		{
			rot = m_rotations[m_numRotKeys - 1];
		}
		else
		{
			bool success = false;
			for (int i = 0; i < m_numRotKeys - 1; i++)
			{
				float time1 = m_rotTimes[i];
				float time2 = m_rotTimes[i + 1];
				if (tick >= time1 && tick < time2)
				{
					rot = Quaternion::Slerp(m_rotations[i], m_rotations[i + 1], GetFractionWithin(tick, time1, time2));
					success = true;
					break;
				}
			}
			if (!success)
				ERROR_RECOVERABLE("Unexpected rotation timeline!");
		}
	}

	if (m_numScaleKeys != 0)
	{
		if (tick < m_scalingTimes[0])
		{
			scale = m_scalings[0];
		}
		else if (tick >= m_scalingTimes[m_numScaleKeys - 1])
		{
			scale = m_scalings[m_numScaleKeys - 1];
		}
		else
		{
			bool success = false;
			for (int i = 0; i < m_numScaleKeys - 1; i++)
			{
				float time1 = m_scalingTimes[i];
				float time2 = m_scalingTimes[i + 1];
				if (tick >= time1 && tick < time2)
				{
					scale = Lerp(m_scalings[i], m_scalings[i + 1], GetFractionWithin(tick, time1, time2));
					success = true;
					break;
				}
			}
			if (!success)
				ERROR_RECOVERABLE("Unexpected scaling timeline!");
		}
	}
}

void AnimationCurve::ReadBytes(ByteBuffer* byteBuf)
{
	char IDENTIFIER[4];
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "ACRV"
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "DATA"
	byteBuf->Read(m_numPosKeys);
	byteBuf->Read(m_numRotKeys);
	byteBuf->Read(m_numScaleKeys);

	Initialize(m_numPosKeys, m_numRotKeys, m_numScaleKeys);
	byteBuf->Read(m_numPosKeys, m_posTimes);
	byteBuf->Read(m_numRotKeys, m_rotTimes);
	byteBuf->Read(m_numScaleKeys, m_scalingTimes);
	byteBuf->Read(m_numPosKeys, m_positions);
	byteBuf->Read(m_numRotKeys, m_rotations);
	byteBuf->Read(m_numScaleKeys, m_scalings);

// 	// Endianness serialization
// 	if (!ByteUtils::IsPlatformBigEndian())
// 	{
// 		for (int i = 0; i < m_numPosKeys; i++)
// 			ByteUtils::ReverseBytes(((int32_t*)m_posTimes)[i]);
// 		
// 		for (int i = 0; i < m_numRotKeys; i++)
// 			ByteUtils::ReverseBytes(((int32_t*)m_rotTimes)[i]);
// 		
// 		for (int i = 0; i < m_numScaleKeys; i++)
// 			ByteUtils::ReverseBytes(((int32_t*)m_scalingTimes)[i]);
// 		
// 		for (int i = 0; i < m_numPosKeys * 3; i++)
// 			ByteUtils::ReverseBytes(((int32_t*)m_positions)[i]);
// 		
// 		for (int i = 0; i < m_numRotKeys * 4; i++)
// 			ByteUtils::ReverseBytes(((int32_t*)m_rotations)[i]);
// 		
// 		for (int i = 0; i < m_numScaleKeys * 4; i++)
// 			ByteUtils::ReverseBytes(((int32_t*)m_scalings)[i]);
// 	}
}

void AnimationCurve::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(4, "ACRV");
	byteBuf->Write(4, "DATA");
//	size_t writeIdx = byteBuf->m_writeIdx;
	byteBuf->Write(m_numPosKeys);
	byteBuf->Write(m_numRotKeys);
	byteBuf->Write(m_numScaleKeys);
	byteBuf->Write(m_numPosKeys, m_posTimes);
	byteBuf->Write(m_numRotKeys, m_rotTimes);
	byteBuf->Write(m_numScaleKeys, m_scalingTimes);
	byteBuf->Write(m_numPosKeys, m_positions);
	byteBuf->Write(m_numRotKeys, m_rotations);
	byteBuf->Write(m_numScaleKeys, m_scalings);

// 	// Endianness serialization
// 	if (!ByteUtils::IsPlatformBigEndian())
// 	{
// 		for (size_t idx = writeIdx; idx < byteBuf->m_writeIdx; idx += 4)
// 			ByteUtils::ReverseBytes32(&byteBuf->data()[idx]);
// 	}
}

AnimationFrame CurveAnimation::Sample(float time, const Pose& defaultPose) const
{
	float tick = time * m_tps;
	tick = fmodf(tick, m_ticks);

	AnimationFrame frame;

	for (int i = 0; i < defaultPose.m_boneLocalPose.size(); i++)
	{
		auto& transform = defaultPose.m_boneLocalPose[i];
		frame.m_positions[i] = transform.m_position;
		frame.m_rotations[i] = transform.m_rotation;
		frame.m_scalings[i]  = transform.m_scaling;
		auto* curve = m_curves[i];
		if (curve)
			curve->ResolveAt(frame.m_positions[i], frame.m_rotations[i], frame.m_scalings[i], tick);
	}

	return frame;
}

void CurveAnimation::ReadBytes(ByteBuffer* byteBuf)
{
	char IDENTIFIER[4];
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "CURV"
	byteBuf->Read(4, &IDENTIFIER[0]); // should read "ANIM"
	Animation::ReadBytes(byteBuf);

	char flags[ENGINE_SKEL_MAX_BONES];

	byteBuf->Read(sizeof(flags), &flags[0]);

	for (int i = 0; i < ENGINE_SKEL_MAX_BONES; i++)
	{
		if (flags[i])
		{
			m_curves[i] = new AnimationCurve();
			m_curves[i]->ReadBytes(byteBuf);
		}
	}
}

void CurveAnimation::WriteBytes(ByteBuffer* byteBuf) const
{
	byteBuf->Write(4, "CURV");
	byteBuf->Write(4, "ANIM");
	Animation::WriteBytes(byteBuf);

	char flags[ENGINE_SKEL_MAX_BONES];

	for (int i = 0; i < ENGINE_SKEL_MAX_BONES; i++)
		flags[i] = m_curves[i] ? 1 : 0;

	byteBuf->Write(sizeof(flags), &flags[0]);
	for (int i = 0; i < ENGINE_SKEL_MAX_BONES; i++)
		if (flags[i])
			m_curves[i]->WriteBytes(byteBuf);
}

AnimationFrame FrameAnimation::Sample(float time, const Pose&) const
{
	int tick = int(time * m_tps);
	tick %= m_frames.size();
	return m_frames[tick];
}

void FrameAnimation::BakeFrom(const Animation& anim, const Pose& defaultPose)
{
	float duration = anim.GetDuration();
	float tpsInv = 1.0f / m_tps;
	int ticks = int(duration * m_tps);
	m_ticks = float(ticks);

	m_frames.resize(ticks);
	for (int tick = 0; tick < ticks; tick++)
	{
		m_frames[tick] = anim.Sample((float)tick * tpsInv, defaultPose);
	}
}

void AnimationFrame::Apply(Pose& pose) const
{
	for (int i = 0; i < pose.m_boneLocalPose.size(); i++)
	{
		auto& transform = pose.m_boneLocalPose[i];
		transform.m_position = m_positions[i];
		transform.m_rotation = m_rotations[i];
		transform.m_scaling = m_scalings[i];
	}
}

void Animation::ReadBytes(ByteBuffer* byteBuf)
{
	ByteUtils::ReadString(byteBuf, m_name);
	byteBuf->Read(m_ticks);
	byteBuf->Read(m_tps);

// 	if (!ByteUtils::IsPlatformBigEndian())
// 	{
// 		ByteUtils::ReverseBytes(*(int32_t*)&m_ticks);
// 		ByteUtils::ReverseBytes(*(int32_t*)&m_tps);
// 	}
}

void Animation::WriteBytes(ByteBuffer* byteBuf) const
{
	ByteUtils::WriteString(byteBuf, m_name);
//	size_t writeIdx = byteBuf->m_writeIdx;
	byteBuf->Write(m_ticks);
	byteBuf->Write(m_tps);

// 	if (!ByteUtils::IsPlatformBigEndian())
// 	{
// 		ByteUtils::ReverseBytes32(&byteBuf->data()[writeIdx]);
// 		ByteUtils::ReverseBytes32(&byteBuf->data()[writeIdx + 4]);
// 	}
}

void FABRIKSolver::Solve(Pose& pose, std::deque<FABRIKNode>* pDebugNodesInitial /*= nullptr*/, std::deque<FABRIKNode>* pDebugNodesAfter /*= nullptr*/)
{
	// let's solve in model's space convention
	const Vec3 effectorPos = m_convMat.GetOrthonormalInverse().TransformPosition3D(m_effector.m_position);

	pose.BakeLocalToComp();
	std::deque<FABRIKNode> nodes;

	if (m_rootBone == INVALID_BONE_ID || m_targetBone == INVALID_BONE_ID)
		return;

	bool chained = false;
	for (BoneId bone = m_targetBone; bone != INVALID_BONE_ID; bone = pose.m_skeleton->FindBone(bone)->m_parentId)
	{
		nodes.push_front(BuildFABRIKNode(pose, bone));
		if (bone == m_rootBone)
		{
			chained = true;
			break;
		}
	}

	if (!chained || !nodes.size())
		return; // root to target not chained or only one bone

	if (nodes.size() < 2)
	{
// 		auto& node = nodes[0];
// 
// 		const Bone* bone   = pose.m_skeleton->FindBone(node.m_boneId);
// 		const Bone* parent = pose.m_skeleton->FindBone(bone->m_parentId);
// 
// 		node.MoveEndTo(effectorPos);
// 		node.MoveStartTo(node.m_preOrigin);
// 
// 		pose.m_boneCompPose[node.m_boneId].m_position = node.m_position;

		return;
	}

	if (pDebugNodesInitial)
		*pDebugNodesInitial = nodes;

	Vec3 start = nodes[0].GetOrigin();
	float length = 0;
	for (auto& node : nodes)
		length += node.m_length;
	if (length * length < (effectorPos - start).GetLengthSquared()) // cannot reach, stretch to a line
	{
		// stretch
		Vec3 origin = start;
		Vec3 direction = (origin - effectorPos).GetNormalized();
		for (int i = 0; i < nodes.size(); i++)
		{
			auto& node = nodes[i];

			origin = origin - direction * node.m_length;
			node.m_position = origin;
			node.m_direction = direction;
		}

		// save nodes
		BuildPose(pose, nodes);
		if (pDebugNodesAfter)
			*pDebugNodesAfter = nodes;
		return;
	}

	// iterate
	for (unsigned int iterate = 0; iterate < m_iterate; iterate++)
	{
		// forward
		Vec3 positionEffector = effectorPos;
		for (int i = (int)nodes.size() - 1; i >= 0; i--)
			positionEffector = nodes[i].MoveEndTo(positionEffector);

		// backward
		positionEffector = nodes[0].m_preOrigin;
		for (size_t i = 0; i < nodes.size(); i++)
			positionEffector = nodes[i].MoveStartTo(positionEffector);

		if ((nodes[0].GetOrigin() - nodes[0].m_preOrigin).GetLengthSquared() > 0.001f)
			ERROR_RECOVERABLE("[FABRIK] Node link origin changed!");

		if ((effectorPos - nodes.back().m_position).GetLengthSquared() < m_tolerance * m_tolerance)
		{
			// tolerance checked
			BuildPose(pose, nodes);
			if (pDebugNodesAfter)
				*pDebugNodesAfter = nodes;
			return;
		}
	}
	// iterate finished
	BuildPose(pose, nodes);
	if (pDebugNodesAfter)
		*pDebugNodesAfter = nodes;
	return;
}

FABRIKNode FABRIKSolver::BuildFABRIKNode(Pose& pose, BoneId boneId)
{
	const Skeleton& skel = *pose.m_skeleton;
	const Bone* bone = skel.FindBone(boneId);

	Vec3 posBoneOrigin = pose.m_boneCompPose[bone->m_parentId].m_position;
	Vec3 posBoneEnd = pose.m_boneCompPose[boneId].m_position;

	return FABRIKNode(boneId, posBoneOrigin, posBoneEnd);
}

void FABRIKSolver::BuildPose(Pose& pose, std::deque<FABRIKNode>& nodes)
{
	auto* skeleton = pose.m_skeleton;
	
	// 1. update position

	for (size_t idx = 0; idx < nodes.size(); idx++)
	{
		auto& node = nodes[idx];
		pose.m_boneCompPose[node.m_boneId].m_position = node.m_position;
	}

	// 2. update rotation 

	// Unreal Engine FABRIK Node

	// save head node
	auto& headNode = nodes.front();
	Quaternion deltaRotHead;
	{
		// Calculate pre-translation vector between this bone and child
		const Vec3 oldDir = (headNode.m_preEnd - headNode.m_preOrigin).GetNormalized();

		// Get vector from the post-translation bone to it's child
		const Vec3 newDir = -headNode.m_direction;

		// Calculate axis of rotation from pre-translation vector to post-translation vector
		const Vec3 rotAxis = oldDir.Cross(newDir).GetNormalized();
		const float rotAngleRad = acosf(oldDir.Dot(newDir));
		deltaRotHead = Quaternion::FromAxisAndAngle(rotAxis, rotAngleRad);
		deltaRotHead.Normalize();
	}

	// FABRIK algorithm - re-orientation of bone local axes after translation calculation
	for (size_t idx = 0; idx < nodes.size() - 1; idx++)
	{
		auto& node = nodes[idx];
		auto& childNode = nodes[idx + 1];

		// Calculate pre-translation vector between this bone and child
		const Vec3 oldDir = (childNode.m_preEnd - childNode.m_preOrigin).GetNormalized();

		// Get vector from the post-translation bone to it's child
		const Vec3 newDir = -childNode.m_direction;

		// Calculate axis of rotation from pre-translation vector to post-translation vector
		const Vec3 rotAxis = oldDir.Cross(newDir).GetNormalized();
		const float rotAngleRad = acosf(oldDir.Dot(newDir));
		Quaternion deltaRot = Quaternion::FromAxisAndAngle(rotAxis, rotAngleRad);
		deltaRot.Normalize();

		// Calculate absolute rotation and set it
		TransformQuat& boneTrans = pose.m_boneCompPose[node.m_boneId];
		boneTrans.m_rotation = Quaternion::FromMatrix(Mat3x3::GetSubMatrix(deltaRot.GetMatrix() * boneTrans.m_rotation.GetMatrix(), 3, 3)); // rotDelta * rotBefore
		boneTrans.m_rotation.Normalize();
	}

	// 3. handle tip bone

	// Unreal Engine FABRIK Node

	size_t lastTip = nodes.size() - 1;
	switch (m_effectorRotHandle)
	{
	case EffectorRotationHandle::KEEP_LOCAL:
	{
		TransformQuat& local = pose.m_boneLocalPose[nodes[lastTip].m_boneId];
		TransformQuat& parent = pose.m_boneCompPose[nodes[lastTip - 1].m_boneId];
		pose.m_boneCompPose[nodes[lastTip].m_boneId].m_rotation = Quaternion::FromMatrix(Mat3x3::GetSubMatrix(local.m_rotation.GetMatrix() * parent.GetMatrix(), 3, 3));
		break;
	}
	case EffectorRotationHandle::COPY:
	{
		pose.m_boneCompPose[nodes[lastTip].m_boneId].m_rotation = Quaternion::FromEuler(m_effector.m_orientation);
		break;
	}
	case EffectorRotationHandle::KEEP_COMP:
	{
		// Don't change the orientation at all
		break;
	}
	default:
		break;
	}

	// 4. bake children

	// save head transform
	TransformQuat transHead = pose.m_boneLocalPose[nodes.front().m_boneId];

	for (size_t idx = 0; idx < nodes.size(); idx++)
	{
		auto& node = nodes[idx];
		BoneId childId = (&node == &nodes.back()) ? INVALID_BONE_ID : nodes[idx + 1].m_boneId;
		auto* bone = skeleton->FindBone(node.m_boneId);

		Mat4x4 boneMat = pose.m_boneCompPose[bone->m_id].GetMatrix();
		for (BoneId child : bone->m_children)
			if (child != childId)
				pose.BakeBoneLocalToComp(child, boneMat);
	}

	// Fix twist bone
	auto* head = skeleton->FindBone(nodes.front().m_boneId);
	auto* parent = skeleton->FindBone(head->m_parentId);

	if (parent)
	{
		const Bone* twist = nullptr;
		for (BoneId childId : parent->m_children)
		{
			auto* child = skeleton->FindBone(childId);
			if (child->m_name.find("twist") != std::string::npos)
			{
				twist = child;
				break;
			}
		}

		if (twist)
		{
			Mat4x4 parentGlobalM = pose.m_boneCompPose[parent->m_id].GetMatrix();
			Mat4x4 boneGlobalM = pose.m_boneCompPose[head->m_id].GetMatrix();

			// AB = C then B = A-1C
			Mat4x4 boneLocalM = parentGlobalM.GetOrthonormalInverse() * boneGlobalM;

			// Calculate head bone local delta rotation
			TransformQuat transHeadAfter = TransformQuat::DecomposeAffineMatrix(boneLocalM);

			// Calculate pre-translation vector between this bone and child
			const Vec3 oldDir = transHead.m_position.GetNormalized();

			// Get vector from the post-translation bone to it's child
			const Vec3 newDir = transHeadAfter.m_position.GetNormalized();

			// Calculate axis of rotation from pre-translation vector to post-translation vector
			const Vec3 rotAxis = oldDir.Cross(newDir).GetNormalized();
			const float rotAngleRad = acosf(oldDir.Dot(newDir));
			Quaternion deltaRot = Quaternion::FromAxisAndAngle(rotAxis, rotAngleRad);
			deltaRot.Normalize();

			TransformQuat& parentTrans = pose.m_boneCompPose[parent->m_id];
			parentTrans.m_rotation = Quaternion::FromMatrix(Mat3x3::GetSubMatrix(deltaRotHead.GetMatrix() * parentTrans.m_rotation.GetMatrix(), 3, 3)); // rotDelta * rotBefore
			parentTrans.m_rotation.Normalize();

			TransformQuat& twistTrans = pose.m_boneLocalPose[twist->m_id];
			twistTrans.m_position = deltaRot.GetMatrix().TransformVectorQuantity3D(twistTrans.m_position);
			twistTrans.m_rotation = Quaternion::FromMatrix(Mat3x3::GetSubMatrix(deltaRot.GetMatrix() * twistTrans.m_rotation.GetMatrix(), 3, 3)); // rotDelta * rotBefore
			twistTrans.m_rotation.Normalize();

			pose.m_boneCompPose[twist->m_id] = TransformQuat::DecomposeAffineMatrix(parentGlobalM * twistTrans.GetMatrix());
		}
	}

	// 5. bake back to local
	
	pose.BakeCompToLocal();
	pose.BakeFromComp();
}

void FABRIKSolver::BakePose(std::deque<FABRIKNode>& nodes, Pose& pose)
{
	pose.m_bakedPose = SkeletonConstants();
	BakeBone(nodes, pose, pose.m_skeleton->GetRoot(), Mat4x4::IDENTITY);
}

void FABRIKSolver::BakeBone(std::deque<FABRIKNode>& nodes, Pose& pose, BoneId boneId, const Mat4x4& parentTransform)
{
	const Bone& bone = *pose.GetSkeleton().FindBone(boneId);

	Mat4x4 nodeTransform = pose.m_boneLocalPose[bone.m_id].GetMatrix();

	Mat4x4 globalTransformation = parentTransform * nodeTransform;

	pose.m_bakedPose[boneId] = globalTransformation * bone.m_offset;

	for (BoneId child : bone.m_children)
		BakeBone(nodes, pose, child, globalTransformation);
}

FABRIKNode::FABRIKNode(BoneId bone, const Vec3& parent, const Vec3& position)
	: m_preOrigin(parent)
	, m_preEnd(position)
	, m_boneId(bone)
	, m_position(position)
{
	m_direction = parent - position; // normalize is done next line
	m_length = m_direction.NormalizeAndGetPreviousLength();
}

Vec3 FABRIKNode::GetOrigin() const
{
	return m_position + m_direction * m_length;
}

Vec3 FABRIKNode::MoveEndTo(const Vec3& effector)
{
	Vec3 origin = GetOrigin();
	m_position = effector;
	m_direction = (origin - effector).GetNormalized();
	return GetOrigin();
}

Vec3 FABRIKNode::MoveStartTo(const Vec3& effector)
{
	m_direction = (effector - m_position).GetNormalized();
	m_position = effector - m_direction * m_length;
	return m_position;
}

void FABRIKNode::GetTransform(Vec3& offset, Quaternion& rotation)
{
	Vec3 origin = GetOrigin();
	offset = origin - m_preOrigin;

	Vec3 preDirection = (m_preEnd - m_preOrigin).GetNormalized();
	Vec3 direction = (m_position - origin).GetNormalized();

	rotation = Quaternion::FromAxisAndAngle(preDirection.Cross(direction), acosf(preDirection.Dot(direction)));
}
