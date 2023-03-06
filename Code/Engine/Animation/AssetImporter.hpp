#include "Engine/Math/Mat4x4.hpp"
#include "Engine/Math/Vec3.hpp"
#include <vector>
#include <string>

struct aiScene;
class Skeleton;
class Animation;
class SkeletalMesh;
struct Mat4x4;

class AssimpRes
{
public:
	AssimpRes(size_t length, const char* data, const char* type = "FBX");
	AssimpRes(const char* path, const char* type = "FBX");
	AssimpRes(const AssimpRes& copyFrom) = delete; // represents memory resource, cannot copy
	~AssimpRes();

	bool                              IsAvaliable() const;
	void                              SetSpaceConventions(const Mat4x4& mat);
	std::vector<SkeletalMesh*>        LoadMesh() const;
	std::vector<Animation*>           LoadAnimation(const Skeleton& skeleton) const;

private:
	const aiScene*    m_pAIScene = nullptr;
	Mat4x4            m_SpaceConv;
};

typedef void(*AsserImporterLogger)(const char* msg, char* user);

class AssetImporterConfig
{
public:
	std::string m_logUser = " Engine";
	AsserImporterLogger m_logger = nullptr;
};

namespace AssetImporter
{
	void           Startup(const AssetImporterConfig& config);
	void           Shutdown();
	void           AssimpSetSpaceConventions(const Mat4x4& mat);
	const aiScene* ImportFile(size_t length, const char* data, const char* type);
	const aiScene* ImportFileFBX(size_t length, const char* data);
	const aiScene* ImportFile(const char* path, const char* type);
	void           ReleaseFileFBX(const aiScene* pAIScene);
	int            ParseMesh(const aiScene* pAIScene, std::vector<SkeletalMesh*>& meshes);
	int            ParseAnimation(const aiScene* pAIScene, const Skeleton& skeleton, std::vector<Animation*>& animations);
};
