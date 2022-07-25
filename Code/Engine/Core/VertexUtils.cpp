#include "VertexUtils.hpp"

#include "Rgba8.hpp"
#include "Vertex_PCU.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/VertexFormat.hpp"

const int VERTNUM_TRIANGLE     = 3;
const int VERTNUM_RECTANGLE    = VERTNUM_TRIANGLE * 2;

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegrees, const Vec2& translate)
{
	for (int vertIdx = 0; vertIdx < numVerts; ++vertIdx)
	{
		Vertex_PCU& vert = verts[vertIdx];
		TransformPositionXY3D(vert.m_position, uniformScaleXY, rotationDegrees, translate);
	}
}

void TransformVertexArray(const Mat4x4& matrix, VertexList& vertices)
{
	for (auto& vert : vertices)
	{
		vert.m_position = matrix.TransformPosition3D(vert.m_position);
	}
}

void AddVertsForBox2D(VertexList& vertsList, Vec2* cornerPoints, const Rgba8& color, AABB2 uvs)
{
	Vertex_PCU boxVerts[4] = {};
	boxVerts[0] = Vertex_PCU(Vec3(cornerPoints[0].x, cornerPoints[0].y, 0.0f), color, Vec2(uvs.m_mins.x, uvs.m_maxs.y));
	boxVerts[1] = Vertex_PCU(Vec3(cornerPoints[1].x, cornerPoints[1].y, 0.0f), color, Vec2(uvs.m_mins.x, uvs.m_mins.y));
	boxVerts[2] = Vertex_PCU(Vec3(cornerPoints[2].x, cornerPoints[2].y, 0.0f), color, Vec2(uvs.m_maxs.x, uvs.m_maxs.y));
	boxVerts[3] = Vertex_PCU(Vec3(cornerPoints[3].x, cornerPoints[3].y, 0.0f), color, Vec2(uvs.m_maxs.x, uvs.m_mins.y));

	vertsList.reserve(vertsList.size() + VERTNUM_RECTANGLE);
	vertsList.push_back(boxVerts[0]);
	vertsList.push_back(boxVerts[1]);
	vertsList.push_back(boxVerts[2]);
	vertsList.push_back(boxVerts[3]);
	vertsList.push_back(boxVerts[2]);
	vertsList.push_back(boxVerts[1]);
}

void AddVertsForCapsule2D(VertexList& vertsList, const Capsule2& capsule, int triangleCount, const Rgba8& color)
{
	int triangleCountOnSector = MaxInt(triangleCount / 2, 2);

	float anglePerTriangle = 180.f / static_cast<float>(triangleCountOnSector);
	Vec3 dir(capsule.m_bone.GetSlopeVectorNormalized().GetRotated90Degrees() * capsule.m_radius);
	Vec3 startSectorPoint0(capsule.m_bone.m_startPos);
	Vec3 endSectorPoint0(capsule.m_bone.m_endPos);

	int vertsNumCapsule = triangleCountOnSector * 2 * VERTNUM_TRIANGLE + VERTNUM_RECTANGLE;
	vertsList.reserve(vertsList.size() + vertsNumCapsule);
	for (int triangleIdx = 0; triangleIdx < triangleCountOnSector; triangleIdx++)
	{
		Vec3 startSectorPoint1 = startSectorPoint0 + dir;
		Vec3 endSectorPoint1 = endSectorPoint0 - dir;
		dir = dir.GetRotatedAboutZDegrees(anglePerTriangle);
		Vec3 startSectorPoint2 = startSectorPoint0 + dir;
		Vec3 endSectorPoint2 = endSectorPoint0 - dir;
		vertsList.push_back(Vertex_PCU(startSectorPoint0, color, Vec2()));
		vertsList.push_back(Vertex_PCU(startSectorPoint1, color, Vec2()));
		vertsList.push_back(Vertex_PCU(startSectorPoint2, color, Vec2()));
		vertsList.push_back(Vertex_PCU(endSectorPoint0, color, Vec2()));
		vertsList.push_back(Vertex_PCU(endSectorPoint1, color, Vec2()));
		vertsList.push_back(Vertex_PCU(endSectorPoint2, color, Vec2()));
	}

	OBB2 boneOBB = OBB2(capsule.m_bone.GetSlopeVectorNormalized(), capsule.m_bone.GetCenter(), Vec2(capsule.m_bone.GetLength() * 0.5f, capsule.m_radius));
	AddVertsForOBB2D(vertsList, boneOBB, color);
}

void AddVertsForCapsule2D(VertexList& vertsList, const Vec2& boneStartPos, const Vec2& boneEndPos, float radius, int triangleCountOnSector, const Rgba8& color)
{
	AddVertsForCapsule2D(vertsList, Capsule2(boneStartPos, boneEndPos, radius), triangleCountOnSector, color);
}

void AddVertsForDisc2D(VertexList& vertsList, const Vec2& center, float radius, int triangleCount, const Rgba8& color, AABB2 uvsAtMinMaxs)
{
	triangleCount = MaxInt(triangleCount, 3);

	float anglePerTriangle = 360.f / static_cast<float>(triangleCount);
	Vec3 dir(radius, 0.0f, 0.0f);
	Vec3 point0(center);

	int vertsNumDisc = triangleCount * VERTNUM_TRIANGLE;
	vertsList.reserve(vertsList.size() + vertsNumDisc);
	for (int triangleIdx = 0; triangleIdx < triangleCount; triangleIdx++)
	{
		Vec3 point1 = dir;
		dir = dir.GetRotatedAboutZDegrees(anglePerTriangle);
		Vec3 point2 = dir;
		vertsList.push_back(Vertex_PCU(point0, color, Vec2()));
		vertsList.push_back(Vertex_PCU(point1 + point0, color, Vec2()));
		vertsList.push_back(Vertex_PCU(point2 + point0, color, Vec2()));
	}
}

void AddVertsForAABB2D(VertexList& vertsList, const AABB2& aabb, const Rgba8& color, AABB2 uvsAtMinMaxs)
{
	Vec2 boxPoints[4] = {};
	boxPoints[0] = Vec2(aabb.m_mins.x, aabb.m_maxs.y);
	boxPoints[1] = Vec2(aabb.m_mins.x, aabb.m_mins.y);
	boxPoints[2] = Vec2(aabb.m_maxs.x, aabb.m_maxs.y);
	boxPoints[3] = Vec2(aabb.m_maxs.x, aabb.m_mins.y);

	AddVertsForBox2D(vertsList, boxPoints, color, uvsAtMinMaxs);
}

void AddVertsForOBB2D(VertexList& vertsList, const OBB2& obb, const Rgba8& color, AABB2 uvsAtMinMaxs)
{
	Vec2 boxPoints[4] = {};
	obb.GetCornerPoints(&boxPoints[0]);

	AddVertsForBox2D(vertsList, boxPoints, color, uvsAtMinMaxs);
}

void AddVertsForLineSegment2D(VertexList& vertsList, const LineSegment2& lineSegment, float width, const Rgba8& color)
{
	AddVertsForOBB2D(vertsList, OBB2(lineSegment.GetSlopeVectorNormalized(), lineSegment.GetCenter(), Vec2(lineSegment.GetLength() + width, width) * 0.5f), color);
}

void AddVertsForLineSegment2D(VertexList& vertsList, const Vec2& startPos, const Vec2& endPos, float width, const Rgba8& color)
{
	AddVertsForLineSegment2D(vertsList, LineSegment2(startPos, endPos), width, color);
}

void AddVertsForArrow2D(VertexList& verts, const Vec2& tailPos, const Vec2& tipPos, float arrowSize, float width, const Rgba8& color)
{
	AddVertsForLineSegment2D(verts, tailPos, tipPos, width, color);
	Vec2 arrowVec = tailPos - tipPos;
	arrowVec.SetLength(arrowSize);
	arrowVec.RotateDegrees(-45.0f);
	AddVertsForLineSegment2D(verts, tipPos + arrowVec, tipPos, width, color);
	arrowVec.Rotate90Degrees();
	AddVertsForLineSegment2D(verts, tipPos + arrowVec, tipPos, width, color);
}

void AddVertsForAABB3D(VertexList& vertsList, const AABB3& aabb, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	vertsList.reserve(vertsList.size() + 3 * 2 * 6);

	// bottom
	Vec2 b0(aabb.m_maxs.x, aabb.m_mins.y);
	Vec2 b1(aabb.m_mins.x, aabb.m_mins.y);
	Vec2 b2(aabb.m_mins.x, aabb.m_maxs.y);
	Vec2 b3(aabb.m_maxs.x, aabb.m_maxs.y);

	Vec3 face[4] = {};

	// make face bottom
	face[0] = Vec3(b0, aabb.m_mins.z);
	face[1] = Vec3(b1, aabb.m_mins.z);
	face[2] = Vec3(b2, aabb.m_mins.z);
	face[3] = Vec3(b3, aabb.m_mins.z);
	// min z
	AddVertsForRect3D(vertsList, &face[0], color, uvsAtMinMaxs);

	// make face top
	face[0] = Vec3(b3, aabb.m_maxs.z);
	face[1] = Vec3(b2, aabb.m_maxs.z);
	face[2] = Vec3(b1, aabb.m_maxs.z);
	face[3] = Vec3(b0, aabb.m_maxs.z);
	// max z
	AddVertsForRect3D(vertsList, &face[0], color, uvsAtMinMaxs);

	// make face side
	face[0] = Vec3(b1, aabb.m_mins.z);
	face[1] = Vec3(b0, aabb.m_mins.z);
	face[2] = Vec3(b0, aabb.m_maxs.z);
	face[3] = Vec3(b1, aabb.m_maxs.z);
	// min y
	AddVertsForRect3D(vertsList, &face[0], color, uvsAtMinMaxs);

	face[0] = Vec3(b2, aabb.m_mins.z);
	face[1] = Vec3(b1, aabb.m_mins.z);
	face[2] = Vec3(b1, aabb.m_maxs.z);
	face[3] = Vec3(b2, aabb.m_maxs.z);
	// max x
	AddVertsForRect3D(vertsList, &face[0], color, uvsAtMinMaxs);

	face[0] = Vec3(b3, aabb.m_mins.z);
	face[1] = Vec3(b2, aabb.m_mins.z);
	face[2] = Vec3(b2, aabb.m_maxs.z);
	face[3] = Vec3(b3, aabb.m_maxs.z);
	// max y
	AddVertsForRect3D(vertsList, &face[0], color, uvsAtMinMaxs);

	face[0] = Vec3(b0, aabb.m_mins.z);
	face[1] = Vec3(b3, aabb.m_mins.z);
	face[2] = Vec3(b3, aabb.m_maxs.z);
	face[3] = Vec3(b0, aabb.m_maxs.z);
	// min x
	AddVertsForRect3D(vertsList, &face[0], color, uvsAtMinMaxs);
}

void AddVertsForSphere(VertexList& vertsList, const Vec3& position, float radius, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/, int grid)
{
	int SPHERE_GRID_Y = grid / 2;
	int SPHERE_GRID_X = SPHERE_GRID_Y * 2;
	int SPHERE_FACES = SPHERE_GRID_X * SPHERE_GRID_Y * 2;
	float SPHERE_GRID_UV_X = 1.0f / (float)SPHERE_GRID_X;
	float SPHERE_GRID_UV_Y = 1.0f / (float)SPHERE_GRID_Y;
	float SPHERE_GRID_ANGLE_X = 360.0f / (float)SPHERE_GRID_X;
	float SPHERE_GRID_ANGLE_Y = 180.0f / (float)SPHERE_GRID_Y;

	vertsList.reserve(SPHERE_FACES);

	for (int x = 0; x < SPHERE_GRID_X; x++)
	{
		for (int y = 0; y < SPHERE_GRID_Y; y++)
		{
			float yaw1   = x * (SPHERE_GRID_ANGLE_X);
			float pitch1 = y * (SPHERE_GRID_ANGLE_Y) - 90.0f;
			float yaw2   = yaw1 + SPHERE_GRID_ANGLE_X;
			float pitch2 = pitch1 + SPHERE_GRID_ANGLE_Y;

			AABB2 uv(SPHERE_GRID_UV_X * (x + 1), 1.0f - SPHERE_GRID_UV_Y * y, SPHERE_GRID_UV_X * x, 1.0f - SPHERE_GRID_UV_Y * (y + 1));

			Vec3 face[4] = {};
			face[0] = EulerAngles(yaw2, pitch1, 0).GetVectorXForward() * radius + position;
			face[1] = EulerAngles(yaw1, pitch1, 0).GetVectorXForward() * radius + position;
			face[2] = EulerAngles(yaw1, pitch2, 0).GetVectorXForward() * radius + position;
			face[3] = EulerAngles(yaw2, pitch2, 0).GetVectorXForward() * radius + position;

			AddVertsForRect3D(vertsList, face, color, uv);
		}
	}
}

void AddVertsForZCylinder(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	constexpr int   CYLINDER_FACES      = 16;
	constexpr int   CYLINDER_VERTNUM    = CYLINDER_FACES * (6 + 3 + 3);
	constexpr float CYLINDER_FACE_UV    = 1.0f / (float)CYLINDER_FACES;
	constexpr float CYLINDER_FACE_ANGLE = 360.0f / (float)CYLINDER_FACES;

	vertsList.reserve(vertsList.size() + CYLINDER_VERTNUM);

	for (int i = 0; i < CYLINDER_FACES; i++)
	{
		float yaw1 = i * (CYLINDER_FACE_ANGLE);
		float yaw2 = yaw1 + CYLINDER_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = Vec3(c1, s1, 0.0f) * radius;
		Vec3 pos2 = Vec3(c2, s2, 0.0f) * radius;

		AABB2 uv(CYLINDER_FACE_UV * float(i), 0.0f, CYLINDER_FACE_UV * float(i + 1), 1.0f);

		Vec3 face[4] = {};
		face[0] = position + pos1;
		face[1] = position + pos2;
		face[2] = position + pos2 + Vec3(0.0f, 0.0f, height);
		face[3] = position + pos1 + Vec3(0.0f, 0.0f, height);

		AddVertsForRect3D(vertsList, face, color, uv);

		Vertex_PCU verts[3] = {};
		verts[0] = Vertex_PCU(position       , color, Vec2(0.5f, 0.5f));
		verts[1] = Vertex_PCU(position + pos2, color, Vec2(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f));
		verts[2] = Vertex_PCU(position + pos1, color, Vec2(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f));

		vertsList.push_back(verts[0]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[2]);

		verts[0] = Vertex_PCU(position       , color, Vec2(0.5f, 0.5f));
		verts[1] = Vertex_PCU(position + pos2, color, Vec2(c2 * 0.5f + 0.5f, s2 * 0.5f + 0.5f));
		verts[2] = Vertex_PCU(position + pos1, color, Vec2(c1 * 0.5f + 0.5f, s1 * 0.5f + 0.5f));

		for (auto& vert : verts)
		{
			vert.m_position.z += height;
		}

		vertsList.push_back(verts[2]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[0]);
	}
}

void AddVertsForXCylinder(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	constexpr int   CYLINDER_FACES = 16;
	constexpr int   CYLINDER_VERTNUM = CYLINDER_FACES * (6 + 3 + 3);
	constexpr float CYLINDER_FACE_UV = 1.0f / (float)CYLINDER_FACES;
	constexpr float CYLINDER_FACE_ANGLE = 360.0f / (float)CYLINDER_FACES;

	vertsList.reserve(vertsList.size() + CYLINDER_VERTNUM);

	for (int i = 0; i < CYLINDER_FACES; i++)
	{
		float yaw1 = i * (CYLINDER_FACE_ANGLE);
		float yaw2 = yaw1 + CYLINDER_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = Vec3(0.0f, c1, s1) * radius;
		Vec3 pos2 = Vec3(0.0f, c2, s2) * radius;

		AABB2 uv(CYLINDER_FACE_UV * float(i), 0.0f, CYLINDER_FACE_UV * float(i + 1), 1.0f);

		Vec3 face[4] = {};
		face[0] = position + pos1;
		face[1] = position + pos2;
		face[2] = position + pos2 + Vec3(height, 0.0f, 0.0f);
		face[3] = position + pos1 + Vec3(height, 0.0f, 0.0f);

		AddVertsForRect3D(vertsList, face, color, uv);

		Vertex_PCU verts[3] = {};
		verts[0] = Vertex_PCU(position, color, Vec2(0.5f, 0.5f));
		verts[1] = Vertex_PCU(position + pos2, color, Vec2(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f));
		verts[2] = Vertex_PCU(position + pos1, color, Vec2(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f));

		vertsList.push_back(verts[0]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[2]);

		verts[0] = Vertex_PCU(position, color, Vec2(0.5f, 0.5f));
		verts[1] = Vertex_PCU(position + pos2, color, Vec2(c2 * 0.5f + 0.5f, s2 * 0.5f + 0.5f));
		verts[2] = Vertex_PCU(position + pos1, color, Vec2(c1 * 0.5f + 0.5f, s1 * 0.5f + 0.5f));

		for (auto& vert : verts)
		{
			vert.m_position.x += height;
		}

		vertsList.push_back(verts[2]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[0]);
	}
}

void AddVertsForZCone(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	constexpr int   CONE_FACES      = 16;
	constexpr int   CONE_VERTNUM    = CONE_FACES * (3 + 3);
	constexpr float CONE_FACE_UV    = 1.0f / (float)CONE_FACES;
	constexpr float CONE_FACE_ANGLE = 360.0f / (float)CONE_FACES;

	vertsList.reserve(vertsList.size() + CONE_VERTNUM);

	for (int i = 0; i < CONE_FACES; i++)
	{
		float yaw1 = i * (CONE_FACE_ANGLE);
		float yaw2 = yaw1 + CONE_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = position + Vec3(c1, s1, 0.0f) * radius;
		Vec3 pos2 = position + Vec3(c2, s2, 0.0f) * radius;

		AABB2 uv(CONE_FACE_UV * float(i), 0.0f, CONE_FACE_UV * float(i + 1), 1.0f);

		Vertex_PCU verts[3] = {};
		verts[0] = Vertex_PCU(position + Vec3(0.0f, 0.0f, height), color, Vec2(uv.GetCenter().x, 0.0f));
		verts[1] = Vertex_PCU(pos1, color, Vec2(uv.m_mins.x, 0.0f));
		verts[2] = Vertex_PCU(pos2, color, Vec2(uv.m_maxs.x, 0.0f));

		vertsList.push_back(verts[0]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[2]);

		verts[0] = Vertex_PCU(position, color, Vec2(0.5f, 0.5f));
		verts[1] = Vertex_PCU(pos2, color, Vec2(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f));
		verts[2] = Vertex_PCU(pos1, color, Vec2(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f));

		vertsList.push_back(verts[0]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[2]);
	}
}

void AddVertsForXCone(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	constexpr int   CONE_FACES = 16;
	constexpr int   CONE_VERTNUM = CONE_FACES * (3 + 3);
	constexpr float CONE_FACE_UV = 1.0f / (float)CONE_FACES;
	constexpr float CONE_FACE_ANGLE = 360.0f / (float)CONE_FACES;

	vertsList.reserve(vertsList.size() + CONE_VERTNUM);

	for (int i = 0; i < CONE_FACES; i++)
	{
		float yaw1 = i * (CONE_FACE_ANGLE);
		float yaw2 = yaw1 + CONE_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = position + Vec3(0.0f, c1, s1) * radius;
		Vec3 pos2 = position + Vec3(0.0f, c2, s2) * radius;

		AABB2 uv(CONE_FACE_UV * float(i), 0.0f, CONE_FACE_UV * float(i + 1), 1.0f);

		Vertex_PCU verts[3] = {};
		verts[0] = Vertex_PCU(position + Vec3(height, 0.0f, 0.0f), color, Vec2(uv.GetCenter().x, 0.0f));
		verts[1] = Vertex_PCU(pos1, color, Vec2(uv.m_mins.x, 0.0f));
		verts[2] = Vertex_PCU(pos2, color, Vec2(uv.m_maxs.x, 0.0f));

		vertsList.push_back(verts[0]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[2]);

		verts[0] = Vertex_PCU(position, color, Vec2(0.5f, 0.5f));
		verts[1] = Vertex_PCU(pos2, color, Vec2(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f));
		verts[2] = Vertex_PCU(pos1, color, Vec2(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f));

		vertsList.push_back(verts[0]);
		vertsList.push_back(verts[1]);
		vertsList.push_back(verts[2]);
	}
}

void AddVertsForRect3D(VertexList& vertsList, const Vec3* face, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	Vertex_PCU rectVerts[4] = {};
	rectVerts[0] = Vertex_PCU(face[0], color, Vec2(uvsAtMinMaxs.m_mins.x, uvsAtMinMaxs.m_mins.y));
	rectVerts[1] = Vertex_PCU(face[1], color, Vec2(uvsAtMinMaxs.m_maxs.x, uvsAtMinMaxs.m_mins.y));
	rectVerts[2] = Vertex_PCU(face[2], color, Vec2(uvsAtMinMaxs.m_maxs.x, uvsAtMinMaxs.m_maxs.y));
	rectVerts[3] = Vertex_PCU(face[3], color, Vec2(uvsAtMinMaxs.m_mins.x, uvsAtMinMaxs.m_maxs.y));

	vertsList.reserve(vertsList.size() + VERTNUM_RECTANGLE);
	vertsList.push_back(rectVerts[0]);
	vertsList.push_back(rectVerts[1]);
	vertsList.push_back(rectVerts[2]);
	vertsList.push_back(rectVerts[2]);
	vertsList.push_back(rectVerts[3]);
	vertsList.push_back(rectVerts[0]);
}


void BuildRect3D(VertexBufferBuilder& builder, const Vec3* face, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	builder.Grow((size_t)VERTNUM_RECTANGLE);
	builder.begin()->pos(face[0])->color(color)->tex(uvsAtMinMaxs.m_mins.x, uvsAtMinMaxs.m_mins.y)->end();
	builder.begin()->pos(face[1])->color(color)->tex(uvsAtMinMaxs.m_maxs.x, uvsAtMinMaxs.m_mins.y)->end();
	builder.begin()->pos(face[2])->color(color)->tex(uvsAtMinMaxs.m_maxs.x, uvsAtMinMaxs.m_maxs.y)->end();
	builder.begin()->pos(face[2])->color(color)->tex(uvsAtMinMaxs.m_maxs.x, uvsAtMinMaxs.m_maxs.y)->end();
	builder.begin()->pos(face[3])->color(color)->tex(uvsAtMinMaxs.m_mins.x, uvsAtMinMaxs.m_maxs.y)->end();
	builder.begin()->pos(face[0])->color(color)->tex(uvsAtMinMaxs.m_mins.x, uvsAtMinMaxs.m_mins.y)->end();
}

void BuildZCylinder(VertexBufferBuilder& builder, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/, bool bottomTop /*= true*/)
{
	constexpr int   CYLINDER_FACES = 16;
	constexpr int   CYLINDER_VERTNUM = CYLINDER_FACES * (6 + 3 + 3);
	constexpr float CYLINDER_FACE_UV = 1.0f / (float)CYLINDER_FACES;
	constexpr float CYLINDER_FACE_ANGLE = 360.0f / (float)CYLINDER_FACES;

	builder.Grow((size_t)CYLINDER_VERTNUM);

	Vec3 positionH = position + Vec3(0.0f, 0.0f, height);

	for (int i = 0; i < CYLINDER_FACES; i++)
	{
		float yaw1 = i * (CYLINDER_FACE_ANGLE);
		float yaw2 = yaw1 + CYLINDER_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = Vec3(c1, s1, 0.0f) * radius;
		Vec3 pos2 = Vec3(c2, s2, 0.0f) * radius;

		AABB2 uv(CYLINDER_FACE_UV * float(i), 0.0f, CYLINDER_FACE_UV * float(i + 1), 1.0f);

		Vec3 face[4] = {};
		face[0] = position + pos1;
		face[1] = position + pos2;
		face[2] = position + pos2 + Vec3(0.0f, 0.0f, height);
		face[3] = position + pos1 + Vec3(0.0f, 0.0f, height);

		BuildRect3D(builder, face, color, uv);

		if (bottomTop)
		{
			builder.begin()->pos(position)->color(color)->tex(0.5f, 0.5f)->end();
			builder.begin()->pos(position + pos2)->color(color)->tex(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f)->end();
			builder.begin()->pos(position + pos1)->color(color)->tex(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f)->end();


			builder.begin()->pos(positionH + pos1)->color(color)->tex(c1 * 0.5f + 0.5f, s1 * 0.5f + 0.5f)->end();
			builder.begin()->pos(positionH + pos2)->color(color)->tex(c2 * 0.5f + 0.5f, s2 * 0.5f + 0.5f)->end();
			builder.begin()->pos(positionH)->color(color)->tex(0.5f, 0.5f)->end();
		}
	}
}

void BuildXCylinder(VertexBufferBuilder& builder, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/, bool bottomTop /*= true*/)
{
	constexpr int   CYLINDER_FACES = 16;
	constexpr int   CYLINDER_VERTNUM = CYLINDER_FACES * (6 + 3 + 3);
	constexpr float CYLINDER_FACE_UV = 1.0f / (float)CYLINDER_FACES;
	constexpr float CYLINDER_FACE_ANGLE = 360.0f / (float)CYLINDER_FACES;

	builder.Grow((size_t)CYLINDER_VERTNUM);

	Vec3 vecH = Vec3(height, 0.0f, 0.0f);
	Vec3 positionH = position + vecH;

	for (int i = 0; i < CYLINDER_FACES; i++)
	{
		float yaw1 = i * (CYLINDER_FACE_ANGLE);
		float yaw2 = yaw1 + CYLINDER_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = Vec3(0.0f, s1, c1) * radius;
		Vec3 pos2 = Vec3(0.0f, s2, c2) * radius;

		AABB2 uv(CYLINDER_FACE_UV * float(i), 0.0f, CYLINDER_FACE_UV * float(i + 1), 1.0f);

		Vec3 face[4] = {};
		face[0] = position + pos2;
		face[1] = position + pos1;
		face[2] = position + pos1 + vecH;
		face[3] = position + pos2 + vecH;

		BuildRect3D(builder, face, color, uv);

		if (bottomTop)
		{
			builder.begin()->pos(position)->color(color)->tex(0.5f, 0.5f)->end();
			builder.begin()->pos(position + pos1)->color(color)->tex(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f)->end();
			builder.begin()->pos(position + pos2)->color(color)->tex(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f)->end();


			builder.begin()->pos(positionH + pos2)->color(color)->tex(c1 * 0.5f + 0.5f, s1 * 0.5f + 0.5f)->end();
			builder.begin()->pos(positionH + pos1)->color(color)->tex(c2 * 0.5f + 0.5f, s2 * 0.5f + 0.5f)->end();
			builder.begin()->pos(positionH)->color(color)->tex(0.5f, 0.5f)->end();
		}
	}
}

void BuildXCone(VertexBufferBuilder& builder, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs /*= AABB2(0.0f, 0.0f, 1.0f, 1.0f)*/)
{
	constexpr int   CONE_FACES = 16;
	constexpr int   CONE_VERTNUM = CONE_FACES * (3 + 3);
	constexpr float CONE_FACE_UV = 1.0f / (float)CONE_FACES;
	constexpr float CONE_FACE_ANGLE = 360.0f / (float)CONE_FACES;

	builder.Grow((size_t)CONE_VERTNUM);

	Vec3 positionH = position + Vec3(height, 0.0f, 0.0f);

	for (int i = 0; i < CONE_FACES; i++)
	{
		float yaw1 = i * (CONE_FACE_ANGLE);
		float yaw2 = yaw1 + CONE_FACE_ANGLE;

		float c1 = CosDegrees(yaw1);
		float s1 = SinDegrees(yaw1);
		float c2 = CosDegrees(yaw2);
		float s2 = SinDegrees(yaw2);

		Vec3 pos1 = position + Vec3(0.0f, c1, s1) * radius;
		Vec3 pos2 = position + Vec3(0.0f, c2, s2) * radius;

		AABB2 uv(CONE_FACE_UV * float(i), 0.0f, CONE_FACE_UV * float(i + 1), 1.0f);

		builder.begin()->pos(positionH)->color(color)->tex(uv.GetCenter().x, 0.0f)->end();
		builder.begin()->pos(pos1)->color(color)->tex(uv.m_mins.x, 0.0f)->end();
		builder.begin()->pos(pos2)->color(color)->tex(uv.m_maxs.x, 0.0f)->end();

		builder.begin()->pos(position)->color(color)->tex(0.5f, 0.5f)->end();
		builder.begin()->pos(pos2)->color(color)->tex(c2 * -0.5f + 0.5f, s2 * 0.5f + 0.5f)->end();
		builder.begin()->pos(pos1)->color(color)->tex(c1 * -0.5f + 0.5f, s1 * 0.5f + 0.5f)->end();
	}
}

