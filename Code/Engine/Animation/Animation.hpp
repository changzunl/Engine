#pragma once

#include "Engine/Animation/Skeleton.hpp"
#include "Engine/Animation/Quaternion.hpp"
#include "Engine/Math/Transformation.hpp"
#include "Engine/Math/Vec3.hpp"
#include <string>
#include <vector>

// ===========================================================================================================
// ===========================================================================================================
class ByteBuffer;
class AnimationFrame;
class CurveAnimation;
typedef std::vector<AnimationFrame> FrameList;


// ===========================================================================================================
// ===========================================================================================================
class AnimationFrame
{
public:
	void Apply(Pose& pose) const;

public:
	Vec3               m_positions[ENGINE_SKEL_MAX_BONES]    = {};
	Quaternion         m_rotations[ENGINE_SKEL_MAX_BONES]    = {};
	Vec3               m_scalings[ENGINE_SKEL_MAX_BONES]     = {};
};


// ===========================================================================================================
// ===========================================================================================================
class Animation
{
public:
	Animation() {}
	virtual ~Animation() {}
	virtual AnimationFrame Sample(float time, const Pose& defaultPose) const = 0;

	inline float GetDuration() const;

	// Serialization
	virtual void ReadBytes(ByteBuffer* byteBuf);
	virtual void WriteBytes(ByteBuffer* byteBuf) const;

public:
	std::string        m_name;
	float              m_ticks = 0;
	float              m_tps = 0;
};


// ===========================================================================================================
// ===========================================================================================================
class FrameAnimation : public Animation
{
public:
	AnimationFrame Sample(float time, const Pose& defaultPose) const override;
	void BakeFrom(const Animation& anim, const Pose& defaultPose);

public:
	FrameList          m_frames;
};


// ===========================================================================================================
// ===========================================================================================================
class AnimationCurve
{
public:
	AnimationCurve();
	AnimationCurve(int numPosKeys, int numRotKeys, int numScaleKeys);
	explicit AnimationCurve(const AnimationCurve& copyFrom);
	~AnimationCurve();

	void Initialize(int numPosKeys, int numRotKeys, int numScaleKeys);
	void ResolveAt(Vec3& pos, Quaternion& rot, Vec3& scale, float tick) const;

	// Serialization
	void ReadBytes(ByteBuffer* byteBuf);
	void WriteBytes(ByteBuffer* byteBuf) const;

public:
	int          m_numPosKeys = 0;
	int          m_numRotKeys = 0;
	int          m_numScaleKeys = 0;
	float*       m_posTimes = nullptr;
	float*       m_rotTimes = nullptr;
	float*       m_scalingTimes = nullptr;
	Vec3*        m_positions = nullptr;
	Quaternion*  m_rotations = nullptr;
	Vec3*        m_scalings = nullptr;
};


// ===========================================================================================================
// ===========================================================================================================
class CurveAnimation : public Animation
{
public:
	AnimationFrame Sample(float time, const Pose& defaultPose) const override;

	// Serialization
	virtual void ReadBytes(ByteBuffer* byteBuf) override;
	virtual void WriteBytes(ByteBuffer* byteBuf) const override;

public:
	AnimationCurve*    m_curves[ENGINE_SKEL_MAX_BONES]       = {};
};

// ===========================================================================================================
//                                              INLINE FUNCTIONS                                                                                          
// ===========================================================================================================
float Animation::GetDuration() const
{
	return m_ticks / m_tps;
}


// ===========================================================================================================
// ===========================================================================================================
struct FABRIKNode
{
public:
	FABRIKNode(BoneId bone, const Vec3& parent, const Vec3& position);

	Vec3 GetOrigin() const;
	Vec3 MoveEndTo(const Vec3& effector);
	Vec3 MoveStartTo(const Vec3& effector);
	void GetTransform(Vec3& offset, Quaternion& rotation);

public:
	BoneId    m_boneId = INVALID_BONE_ID;

	Vec3      m_preOrigin;
	Vec3      m_preEnd;

	Vec3      m_position;
	float     m_length = 0.0f;
	Vec3      m_direction = -Vec3::ONE;
};


// ===========================================================================================================
//                                                IK SOLVERS
// ===========================================================================================================
#include <deque>


// ===========================================================================================================
// ===========================================================================================================
enum class EffectorRotationHandle
{
	KEEP_LOCAL,
	COPY,              // Copy target rotation
	KEEP_COMP,         // 
};


// ===========================================================================================================
// ===========================================================================================================
class FABRIKSolver
{
public:
	Mat4x4 m_convMat;
	EffectorRotationHandle m_effectorRotHandle = EffectorRotationHandle::KEEP_LOCAL;
	Transformation m_effector;
	BoneId m_rootBone = INVALID_BONE_ID;
	BoneId m_targetBone = INVALID_BONE_ID;
	unsigned int m_iterate = 10;
	float m_tolerance = 0.1f;

public:
	void        Solve(Pose& pose, std::deque<FABRIKNode>* pDebugNodesInitial = nullptr, std::deque<FABRIKNode>* pDebugNodesAfter = nullptr);

private:
	FABRIKNode  BuildFABRIKNode(Pose& pose, BoneId bone);
	void        BuildPose(Pose& pose, std::deque<FABRIKNode>& nodes);
	void        BakePose(std::deque<FABRIKNode>& nodes, Pose& pose);
	void        BakeBone(std::deque<FABRIKNode>& nodes, Pose& pose, BoneId boneId, const Mat4x4& transform);
};

