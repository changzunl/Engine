#pragma once

#include "Engine/Math/AABB2.hpp"

#include <vector>

struct AABB3;
struct Vec2;
struct Vec3;
struct LineSegment2;
struct Capsule2;
struct OBB2;
struct Rgba8;
struct Mat4x4;
struct Vertex_PCU;
class CubicHermiteCurve2D;
class VertexBufferBuilder;

typedef std::vector<Vertex_PCU> VertexList;

void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float uniformScaleXY, float rotationDegrees, const Vec2& translate);
void TransformVertexArray(const Mat4x4& matrix, VertexList& vertices);

void AddVertsForBox2D(VertexList& vertsList, Vec2* cornerPoints, const Rgba8& color, AABB2 uvs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForCapsule2D(VertexList& vertsList, const Capsule2& capsule, int triangleCountOnSector, const Rgba8& color);
void AddVertsForCapsule2D(VertexList& vertsList, const Vec2& boneStartPos, const Vec2& boneEndPos, float radius, int triangleCount, const Rgba8& color);
void AddVertsForDisc2D(VertexList& vertsList, const Vec2& center, float radius, int triangleCount, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForAABB2D(VertexList& vertsList, const AABB2& aabb, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForOBB2D(VertexList& vertsList, const OBB2& obb, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForLineSegment2D(VertexList& vertsList, const LineSegment2& lineSegment, float width, const Rgba8& color);
void AddVertsForLineSegment2D(VertexList& vertsList, const Vec2& startPos, const Vec2& endPos, float width, const Rgba8& color);
void AddVertsForArrow2D(VertexList& verts, const Vec2& tailPos, const Vec2& tipPos, float arrowSize, float width, const Rgba8& color);
void AddVertsForHermite2D(VertexList& verts, const CubicHermiteCurve2D& curve, float width, const Rgba8& color);

void AddVertsForAABB3D(VertexList& vertsList, const AABB3& aabb, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForSphere(VertexList& vertsList, const Vec3& position, float radius, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f), int grid = 32);
void AddVertsForZCylinder(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForXCylinder(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForZCone(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForXCone(VertexList& vertsList, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void AddVertsForRect3D(VertexList& vertsList, const Vec3* face, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));

void BuildRect3D(VertexBufferBuilder& builder, const Vec3* face, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));
void BuildZCylinder(VertexBufferBuilder& builder, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f), bool bottomTop = true);
void BuildXCylinder(VertexBufferBuilder& builder, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f), bool bottomTop = true);
void BuildXCone(VertexBufferBuilder& builder, const Vec3& position, float radius, float height, const Rgba8& color, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f));

void AddVertsForUIBox(VertexList& vertsList, const AABB2& aabb, const Rgba8& color, float corner = 0.1f, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f), float uvCorner = 0.1f);
void AddVertsForUIFrame(VertexList& vertsList, const AABB2& aabb, const Rgba8& color, float corner = 0.1f, AABB2 uvsAtMinMaxs = AABB2(0.0f, 0.0f, 1.0f, 1.0f), float uvCorner = 0.1f);
