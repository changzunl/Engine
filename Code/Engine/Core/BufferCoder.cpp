#include "Engine/Core/BufferCoder.hpp"


std::string BufferCoder::ReadCString(IOBuffer& buffer)
{
    std::string str = std::string((const char*)buffer.m_buffer.data()[buffer.m_buffer.m_readIdx]);
    buffer.skip(str.size() + 1);
    return str;
}

std::string BufferCoder::ReadString(IOBuffer& buffer)
{
    size_t size = buffer.ReadUInt();
    std::string str;
    str.resize(size, 'E');
    buffer.read(size, str.data());
    return str;
}

Vec2 BufferCoder::ReadVec2(IOBuffer& buffer)
{
    return Vec2(buffer.ReadFloat(), buffer.ReadFloat());
}

Vec3 BufferCoder::ReadVec3(IOBuffer& buffer)
{
    return Vec3(buffer.ReadFloat(), buffer.ReadFloat(), buffer.ReadFloat());
}

Vec4 BufferCoder::ReadVec4(IOBuffer& buffer)
{
    return Vec4(buffer.ReadFloat(), buffer.ReadFloat(), buffer.ReadFloat(), buffer.ReadFloat());
}

IntVec2 BufferCoder::ReadIntVec2(IOBuffer& buffer)
{
    return IntVec2(buffer.ReadInt(), buffer.ReadInt());
}

IntVec3 BufferCoder::ReadIntVec3(IOBuffer& buffer)
{
    return IntVec3(buffer.ReadInt(), buffer.ReadInt(), buffer.ReadInt());
}

Rgba8 BufferCoder::ReadRgba8(IOBuffer& buffer)
{
    return Rgba8(buffer.ReadByte(), buffer.ReadByte(), buffer.ReadByte(), buffer.ReadByte());
}

AABB2 BufferCoder::ReadAABB2(IOBuffer& buffer)
{
    AABB2 aabb;
    aabb.m_mins = ReadVec2(buffer);
    aabb.m_maxs = ReadVec2(buffer);

    return aabb;
}

AABB3 BufferCoder::ReadAABB3(IOBuffer& buffer)
{
    AABB3 aabb;
    aabb.m_mins = ReadVec3(buffer);
    aabb.m_maxs = ReadVec3(buffer);

    return aabb;
}

Vertex_PCU BufferCoder::ReadVertex_PCU(IOBuffer& buffer)
{
    Vertex_PCU v;
    v.m_position = ReadVec3(buffer);
    v.m_color = ReadRgba8(buffer);
    v.m_uvTexCoords = ReadVec2(buffer);

    return v;
}

void BufferCoder::Write(IOBuffer& buffer, const char* val)
{
    // read size first
    uint32_t size = 1;
    const char* ptr = val;
    while (*ptr)
    {
        size++;
        ptr++;
    }

    buffer.write(size, val);
}

void BufferCoder::Write(IOBuffer& buffer, const std::string& val)
{
    uint32_t size = (uint32_t)val.size();
    buffer.WriteUInt(size);
    buffer.write(size, val.data());
}

void BufferCoder::Write(IOBuffer& buffer, const Vec2& val)
{
    buffer.WriteFloat(val.x);
    buffer.WriteFloat(val.y);
}

void BufferCoder::Write(IOBuffer& buffer, const Vec3& val)
{
    buffer.WriteFloat(val.x);
    buffer.WriteFloat(val.y);
    buffer.WriteFloat(val.z);
}

void BufferCoder::Write(IOBuffer& buffer, const Vec4& val)
{
    buffer.WriteFloat(val.x);
    buffer.WriteFloat(val.y);
    buffer.WriteFloat(val.z);
    buffer.WriteFloat(val.w);
}

void BufferCoder::Write(IOBuffer& buffer, const IntVec2& val)
{
    buffer.WriteInt(val.x);
    buffer.WriteInt(val.y);
}

void BufferCoder::Write(IOBuffer& buffer, const IntVec3& val)
{
    buffer.WriteInt(val.x);
    buffer.WriteInt(val.y);
    buffer.WriteInt(val.z);
}

void BufferCoder::Write(IOBuffer& buffer, const Rgba8& val)
{
    buffer.WriteChar(val.r);
    buffer.WriteChar(val.g);
    buffer.WriteChar(val.b);
    buffer.WriteChar(val.a);
}

void BufferCoder::Write(IOBuffer& buffer, const AABB2& val)
{
    Write(buffer, val.m_mins);
    Write(buffer, val.m_maxs);
}

void BufferCoder::Write(IOBuffer& buffer, const AABB3& val)
{
    Write(buffer, val.m_mins);
    Write(buffer, val.m_maxs);
}

void BufferCoder::Write(IOBuffer& buffer, const Vertex_PCU& val)
{
    Write(buffer, val.m_position);
    Write(buffer, val.m_color);
    Write(buffer, val.m_uvTexCoords);
}
