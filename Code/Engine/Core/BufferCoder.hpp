#pragma once

#include "Engine/Core/IOBuffer.hpp"

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <string>

class BufferCoder
{
public:
	std::string       ReadCString(IOBuffer& buffer);
    std::string       ReadString(IOBuffer& buffer);
    Vec2              ReadVec2(IOBuffer& buffer);
    Vec3              ReadVec3(IOBuffer& buffer);
    Vec4              ReadVec4(IOBuffer& buffer);
    IntVec2           ReadIntVec2(IOBuffer& buffer);
    IntVec3           ReadIntVec3(IOBuffer& buffer);
    Rgba8             ReadRgba8(IOBuffer& buffer);
    AABB2             ReadAABB2(IOBuffer& buffer);
    AABB3             ReadAABB3(IOBuffer& buffer);
    Vertex_PCU        ReadVertex_PCU(IOBuffer& buffer);

    void              Write(IOBuffer& buffer, const char* val);
    void              Write(IOBuffer& buffer, const std::string& val);
    void              Write(IOBuffer& buffer, const Vec2& val);
    void              Write(IOBuffer& buffer, const Vec3& val);
    void              Write(IOBuffer& buffer, const Vec4& val);
    void              Write(IOBuffer& buffer, const IntVec2& val);
    void              Write(IOBuffer& buffer, const IntVec3& val);
    void              Write(IOBuffer& buffer, const Rgba8& val);
    void              Write(IOBuffer& buffer, const AABB2& val);
    void              Write(IOBuffer& buffer, const AABB3& val);
    void              Write(IOBuffer& buffer, const Vertex_PCU& val);
};

