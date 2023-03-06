#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat4x4.hpp"

#include <string>
#include <vector>

#define ENGINE_MESH_MAX_UVCHANNELS 8

class ByteBuffer;

class Mesh
{
protected:
	Mesh() {};
	Mesh(const char* name) : m_name(name) {};
	virtual ~Mesh() {};

public:
	// Serialization
	virtual void ReadBytes(ByteBuffer* byteBuf);
	virtual void WriteBytes(ByteBuffer* byteBuf) const;

	virtual void TransformBasis(const Vec3& i, const Vec3& j, const Vec3& k);
	virtual void ScaleMesh(float scale);
	virtual void FlipUV();
	virtual void ReverseWindingOrder();

public:
	std::string         m_name;
	Mat4x4              m_transformation;

	std::vector<Vec3>   m_vertices;
	std::vector<Vec3>   m_normals;
	std::vector<Vec2>   m_uvs[ENGINE_MESH_MAX_UVCHANNELS];
	std::vector<int>    m_indices;
};

class StaticMesh : public Mesh
{
protected:
	StaticMesh() {};
	StaticMesh(const char* name) : Mesh(name) {};
	virtual ~StaticMesh() {};

	// Serialization
	virtual void ReadBytes(ByteBuffer* byteBuf) override;
	virtual void WriteBytes(ByteBuffer* byteBuf) const override;
};

