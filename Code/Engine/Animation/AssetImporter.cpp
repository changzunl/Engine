#include "AssetImporter.hpp"

//-----------------------------------------------------------------------------------------------
// To enable support for model and animation asset import (and add requirement for assimp.dll) for any game,
//	#define ENGINE_ENABLE_ASSIMP in your game's Code/Game/EngineBuildPreferences.hpp file.
//
// Note that this #include is an exception to the rule "engine code doesn't know about game code".
//	Purpose: Each game can now direct the engine via #defines to build differently for that game.
//	Downside: ALL games must now have this Code/Game/EngineBuildPreferences.hpp file.
//
// SD1 NOTE: THIS MEANS *EVERY* GAME MUST HAVE AN EngineBuildPreferences.hpp FILE IN ITS CODE/GAME FOLDER!!
#include "Game/EngineBuildPreferences.hpp"
#if defined( ENGINE_ENABLE_ASSIMP )

#include "Engine/Animation/Animation.hpp"
#include "Engine/Animation/SkeletalMesh.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "ThirdParty/assimp/scene.h"
#include "ThirdParty/assimp/mesh.h"
#include "ThirdParty/assimp/cimport.h"
#include "ThirdParty/assimp/postprocess.h"

#pragma comment( lib, "ThirdParty/assimp/assimp-vc142-mt.lib" )


namespace AssetImporter
{
	AssetImporterConfig g_config;
	aiLogStream g_logStream;
	Mat4x4 g_SpaceConv = Mat4x4::IDENTITY;

	int           ParseMesh(const aiScene* pAIScene, const aiNode* pAINode, std::vector<SkeletalMesh*>& meshes);
	void          ParseMesh(const aiNode* pAINode, const aiMesh* pAIMesh, SkeletalMesh& mesh);
	void          ParseSkeleton(const aiNode* pAINode, const aiMesh* pAIMesh, SkeletalMesh& mesh);
	void          ParseAnimation(const aiAnimation* pAIAnimation, const Skeleton& skeleton, Animation*& animation);

	Mat4x4        ParseTransformation(const aiNode* pAINode);
	Mat4x4        Convert(const aiMatrix4x4& aiVal);
	Vec2          Convert(const aiVector2D& aiVal);
	Vec3          ConvertAbsolute(const aiVector3D& aiVal);
	Vec3          Convert(const aiVector3D& aiVal);
	Quaternion    Convert(const aiQuaternion& aiVal);
	Vec2          ConvertUV(const aiVector3D& aiVal);
};


void AssetImporter::Startup(const AssetImporterConfig& config)
{
	g_config = config;
	g_logStream.callback = g_config.m_logger;
	g_logStream.user = &g_config.m_logUser[0];
	aiAttachLogStream(&g_logStream);
}

void AssetImporter::Shutdown()
{
	aiDetachLogStream(&g_logStream);
}

void AssetImporter::AssimpSetSpaceConventions(const Mat4x4& mat)
{
	g_SpaceConv = mat.GetOrthonormalInverse();
}

const aiScene* AssetImporter::ImportFile(size_t length, const char* data, const char* type)
{
	return aiImportFileFromMemory(data, (uint32_t)length, aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData, type);
}

void AssetImporter::ReleaseFileFBX(const aiScene* pAIScene)
{
	aiReleaseImport(pAIScene);
}

const aiScene* AssetImporter::ImportFileFBX(size_t length, const char* data)
{
	return ImportFile(length, data, "FBX");
}

const aiScene* AssetImporter::ImportFile(const char* path, const char* type)
{
	std::vector<uint8_t> buffer;
	FileReadToBuffer(buffer, path);
	return ImportFile(buffer.size(), (char*)buffer.data(), type);
}

int AssetImporter::ParseMesh(const aiScene* pAIScene, std::vector<SkeletalMesh*>& meshes)
{
	return ParseMesh(pAIScene, pAIScene->mRootNode, meshes);
}

int AssetImporter::ParseAnimation(const aiScene* pAIScene, const Skeleton& skeleton, std::vector<Animation*>& animations)
{
	animations.reserve(animations.size() + pAIScene->mNumAnimations);
	for (unsigned int i = 0; i < pAIScene->mNumAnimations; i++)
	{
		Animation* animation;
		ParseAnimation(pAIScene->mAnimations[i], skeleton, animation);
		animations.push_back(animation);
	}
	return (int)pAIScene->mNumAnimations;
}

int AssetImporter::ParseMesh(const aiScene* pAIScene, const aiNode* pAINode, std::vector<SkeletalMesh*>& meshes)
{
	int size = 0;
	// recursive parse child nodes
	for (size_t i = 0; i < pAINode->mNumChildren; i++)
		size += ParseMesh(pAIScene, pAINode->mChildren[i], meshes);

	// parse mesh in current node
	for (size_t i = 0; i < pAINode->mNumMeshes; i++)
	{
		SkeletalMesh* mesh = new SkeletalMesh();
		ParseMesh(pAINode, pAIScene->mMeshes[pAINode->mMeshes[i]], *mesh);
		meshes.push_back(mesh);
		size++;
	}
	return size;
}

void AssetImporter::ParseMesh(const aiNode* pAINode, const aiMesh* pAIMesh, SkeletalMesh& mesh)
{
	if (pAIMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
		ERROR_AND_DIE("Not triangulated");

	// 0. metadata

	// parse name
	mesh.m_name = pAIMesh->mName.C_Str();
	mesh.m_transformation = ParseTransformation(pAINode);


	// 1. vertex elements

	// parse vertices
	mesh.m_vertices.resize(pAIMesh->mNumVertices);
	for (size_t i = 0; i < pAIMesh->mNumVertices; i++)
		mesh.m_vertices[i] = Convert(pAIMesh->mVertices[i]);

	//parse normals
	mesh.m_normals.resize(pAIMesh->mNumVertices);
	for (size_t i = 0; i < pAIMesh->mNumVertices; i++)
		mesh.m_normals[i] = Convert(pAIMesh->mNormals[i]);

	// reserve bone weights
	mesh.m_boneIndices.resize(pAIMesh->mNumVertices);
	mesh.m_boneWeights.resize(pAIMesh->mNumVertices);

	// parse uv channels
	for (int uvChn = 0; uvChn < AI_MAX_NUMBER_OF_TEXTURECOORDS; uvChn++)
	{
		aiVector3D* uvs = pAIMesh->mTextureCoords[uvChn];
		switch (pAIMesh->mNumUVComponents[uvChn])
		{
		case 2:
			mesh.m_uvs[uvChn].reserve(pAIMesh->mNumVertices);
			for (size_t vertIdx = 0; vertIdx < pAIMesh->mNumVertices; vertIdx++)
				mesh.m_uvs[uvChn].push_back(ConvertUV(uvs[vertIdx]));
			break;
		case 0:
			break;
		default:
			ERROR_AND_DIE("Unsupported uv channel");
		}
	}


	// 2. index data

	//parse indices
	mesh.m_indices.resize(3ULL * pAIMesh->mNumFaces);
	for (size_t faceIdx = 0; faceIdx < pAIMesh->mNumFaces; faceIdx++)
	{
		const aiFace* pAIFace = &pAIMesh->mFaces[faceIdx];
		memcpy(&mesh.m_indices[faceIdx * 3], pAIFace->mIndices, sizeof(int) * 3);
	}


	// 3. skeleton data

	// parse skeletal vertex elements
	for (unsigned int boneIdx = 0; boneIdx < pAIMesh->mNumBones; boneIdx++)
	{
		const aiBone* pAIBone = pAIMesh->mBones[boneIdx];

		// parse bone weights
		for (unsigned int boneWtIdx = 0; boneWtIdx < pAIBone->mNumWeights; boneWtIdx++)
		{
			const aiVertexWeight* pAIVertWt = &pAIBone->mWeights[boneWtIdx];
			int vertId = pAIVertWt->mVertexId;
			float vertWt = pAIVertWt->mWeight;
			BoneIndices& bIdx = mesh.m_boneIndices[vertId];
			BoneWeights& bWts = mesh.m_boneWeights[vertId];
			for (int vertWtIdx = 0; vertWtIdx < ENGINE_SKEL_MAX_BONES; vertWtIdx++)
			{
				if (bWts.weights[vertWtIdx] == 0.0f)
				{
					bIdx.indices[vertWtIdx] = (unsigned char)boneIdx;
					bWts.weights[vertWtIdx] = vertWt;
					break;
				}
				if (vertWtIdx == ENGINE_SKEL_MAX_BONE_WEIGHTS - 1)
				{
					ERROR_AND_DIE("Vertex weight exceeds max count!");
				}
			}
		}
	}

	// parse skeleton
	ParseSkeleton(pAINode, pAIMesh, mesh);
}

void AssetImporter::ParseSkeleton(const aiNode* pAINode, const aiMesh* pAIMesh, SkeletalMesh& mesh)
{
	UNUSED(pAINode);

	Skeleton& skeleton = mesh.m_skeleton;
	skeleton.SetBones(pAIMesh->mNumBones);

	aiNode* boneNodes[ENGINE_SKEL_MAX_BONES];

	// parse bone
	for (BoneId boneIdx = 0; boneIdx < pAIMesh->mNumBones; boneIdx++)
	{
		const aiBone* pAIBone = pAIMesh->mBones[boneIdx];

		Bone& bone = *skeleton.FindBone(boneIdx);

		// parse bone data
		boneNodes[boneIdx] = pAIBone->mNode;
		bone.m_id = boneIdx;
		bone.m_name = pAIBone->mName.C_Str();
		bone.m_transform = Convert(pAIBone->mNode->mTransformation);
		bone.m_offset = Convert(pAIBone->mOffsetMatrix);
	}

	// Initialize default pose
	skeleton.GetPose().Reset();

	// parse bone hierarchy
	for (BoneId boneIdx = 0; boneIdx < pAIMesh->mNumBones; boneIdx++)
	{
		const aiBone* pAIBone = pAIMesh->mBones[boneIdx];

		// parse bone parent
		aiNode* pAIBoneNode = pAIBone->mNode->mParent;
		for (BoneId bIdx = 0; bIdx < pAIMesh->mNumBones; bIdx++)
		{
			if (pAIBoneNode == boneNodes[bIdx])
			{
				skeleton.FindBone(boneIdx)->m_parentId = bIdx;
				skeleton.FindBone(bIdx)->m_children.push_back(boneIdx);
			}
		}
	}

	// find bone root
	for (const auto& bone : skeleton)
	{
		if (bone.m_parentId == INVALID_BONE_ID)
		{
			ASSERT_OR_DIE(skeleton.GetRoot() == INVALID_BONE_ID, "Not nested bone detected");
			skeleton.SetRoot(bone.m_id);
		}
	}
}

void AssetImporter::ParseAnimation(const aiAnimation* pAIAnimation, const Skeleton& skeleton, Animation*& animation)
{
	CurveAnimation* canim = new CurveAnimation();

	canim->m_name = pAIAnimation->mName.C_Str();
	canim->m_ticks = (float)pAIAnimation->mDuration;
	canim->m_tps = pAIAnimation->mTicksPerSecond ? (float)pAIAnimation->mTicksPerSecond : 1.0f;

	for (unsigned int chn = 0; chn < pAIAnimation->mNumChannels; chn++)
	{
		auto* pAIChannel = pAIAnimation->mChannels[chn];
		
		BoneId boneId = skeleton.FindBone(pAIChannel->mNodeName.C_Str());
		if (boneId == INVALID_BONE_ID)
		{
			g_config.m_logger(Stringf("Invalid animation channel: %s", pAIChannel->mNodeName.C_Str()).c_str(), &g_config.m_logUser[0]);
			continue;
		}

		auto& curve = canim->m_curves[boneId];

		if (curve)
		{
			g_config.m_logger(Stringf("Duplicate animation channel: %s", pAIChannel->mNodeName.C_Str()).c_str(), &g_config.m_logUser[0]);
			delete curve;
		}

		curve = new AnimationCurve(pAIChannel->mNumPositionKeys, pAIChannel->mNumRotationKeys, pAIChannel->mNumScalingKeys);

		for (int i = 0; i < curve->m_numPosKeys; i++)
		{
			curve->m_posTimes[i]  = (float)pAIChannel->mPositionKeys[i].mTime;
			curve->m_positions[i] = Convert(pAIChannel->mPositionKeys[i].mValue);
		}

		for (int i = 0; i < curve->m_numRotKeys; i++)
		{
			curve->m_rotTimes[i] = (float)pAIChannel->mRotationKeys[i].mTime;
			curve->m_rotations[i] = Convert(pAIChannel->mRotationKeys[i].mValue);
		}

		for (int i = 0; i < curve->m_numScaleKeys; i++)
		{
			curve->m_scalingTimes[i] = (float)pAIChannel->mScalingKeys[i].mTime;
			curve->m_scalings[i] = Convert(pAIChannel->mScalingKeys[i].mValue);
		}
	}

	animation = canim;
}

Mat4x4 AssetImporter::ParseTransformation(const aiNode* pAINode)
{
	std::vector<Mat4x4> matrices; // stores chain transformation in reverse order
	while (pAINode)
	{
		matrices.push_back(Convert(pAINode->mTransformation));
		pAINode = pAINode->mParent;
	}

	Mat4x4 matrix;
	for (auto& mat : matrices)
		matrix.Append(mat);
	return matrix;
}

Mat4x4 AssetImporter::Convert(const aiMatrix4x4& aiVal)
{
	Mat4x4 mat = Mat4x4(&aiVal.a1);
	mat.Transpose(); // assimp uses ROW-MAJOR matrix layout

	Mat4x4 matOut;
	// matOut.Append(g_Assimp_SpaceConv); // assimp to game
	matOut.Append(mat);
	// matOut.Append(g_Assimp_SpaceConv.GetOrthonormalInverse()); // game to assimp
	return matOut;
}

Vec2 AssetImporter::Convert(const aiVector2D& aiVal)
{
	Vec2 vec = Vec2(aiVal.x, aiVal.y);
	return vec; // don't care space conventions in 2d space
}

Vec3 AssetImporter::ConvertAbsolute(const aiVector3D& aiVal)
{
	return Vec3(aiVal.x, aiVal.y, aiVal.z);
}

Vec3 AssetImporter::Convert(const aiVector3D& aiVal)
{
	Vec3 vec = Vec3(aiVal.x, aiVal.y, aiVal.z);
	return g_SpaceConv.TransformPosition3D(vec); // assimp to game
}


Quaternion AssetImporter::Convert(const aiQuaternion& aiVal)
{
	Vec3 vec = Vec3(aiVal.x, aiVal.y, aiVal.z);
	vec = g_SpaceConv.TransformPosition3D(vec);
	Quaternion quat = Quaternion(vec.x, vec.y, vec.z, aiVal.w);
	return quat;
}

Vec2 AssetImporter::ConvertUV(const aiVector3D& aiVal)
{
	return Vec2(aiVal.x, aiVal.y);
}


AssimpRes::AssimpRes(size_t length, const char* data, const char* type/* = "FBX"*/)
	: m_pAIScene(AssetImporter::ImportFile(length, data, type))
	, m_SpaceConv(AssetImporter::g_SpaceConv)
{
}

AssimpRes::AssimpRes(const char* path, const char* type/* = "FBX"*/)
	: m_pAIScene(AssetImporter::ImportFile(path, type))
	, m_SpaceConv(AssetImporter::g_SpaceConv)
{
}

AssimpRes::~AssimpRes()
{
	if (m_pAIScene)
		AssetImporter::ReleaseFileFBX(m_pAIScene);
}

void AssimpRes::SetSpaceConventions(const Mat4x4& mat)
{
	m_SpaceConv = mat.GetOrthonormalInverse();
}

std::vector<SkeletalMesh*> AssimpRes::LoadMesh() const
{
	std::vector<SkeletalMesh*> meshes;

	Mat4x4 matOriginal = AssetImporter::g_SpaceConv;
	AssetImporter::g_SpaceConv = m_SpaceConv;
	AssetImporter::ParseMesh(m_pAIScene, meshes);
	AssetImporter::g_SpaceConv = matOriginal;
	return meshes;
}

std::vector<Animation*> AssimpRes::LoadAnimation(const Skeleton& skeleton) const
{
	std::vector<Animation*> animations;
	Mat4x4 matOriginal = AssetImporter::g_SpaceConv;
	AssetImporter::g_SpaceConv = m_SpaceConv;
	AssetImporter::ParseAnimation(m_pAIScene, skeleton, animations);
	AssetImporter::g_SpaceConv = matOriginal;
	return animations;
}

#endif // defined( ENGINE_ENABLE_ASSIMP )

