#include "Engine/Core/IOBuffer.hpp"

constexpr bool IsBigEndian()
{
    union
    {
        int i = 0x01020304;
        char b;
    } const un;

    return un.b == 0x01;
}

IOBuffer::IOBuffer(size_t initialCapacity)
    : m_buffer(initialCapacity)
{
}

BYTE              IOBuffer::ReadByte()
{
    BYTE val;
    m_buffer.Read(val);
    return val;
}

char              IOBuffer::ReadChar()
{
    char val;
    m_buffer.Read(val);
    return val;
}

int16_t           IOBuffer::ReadShort()
{
    int16_t val;
    m_buffer.Read(val);
    flip16(&val);
    return val;
}

uint16_t          IOBuffer::ReadUShort()
{
    uint16_t val;
    m_buffer.Read(val);
    flip16(&val);
    return val;
}

int32_t           IOBuffer::ReadInt()
{
    int32_t val;
    m_buffer.Read(val);
    flip32(&val);
    return val;
}

uint32_t          IOBuffer::ReadUInt()
{
    uint32_t val;
    m_buffer.Read(val);
    flip32(&val);
    return val;
}

int64_t           IOBuffer::ReadLong()
{
    int64_t val;
    m_buffer.Read(val);
    flip64(&val);
    return val;
}

uint64_t          IOBuffer::ReadULong()
{
    uint64_t val;
    m_buffer.Read(val);
    flip64(&val);
    return val;
}

float             IOBuffer::ReadFloat()
{
    float val;
    m_buffer.Read(val);
    flip32(&val);
    return val;
}

double            IOBuffer::ReadDouble()
{
    double val;
    m_buffer.Read(val);
    flip64(&val);
    return val;
}

size_t            IOBuffer::ReadSize32()
{
    return (size_t)ReadUInt();
}

size_t            IOBuffer::ReadSize64()
{
    return (size_t)ReadUInt();
}


void IOBuffer::WriteByte(BYTE val)
{
    m_buffer.Write(val);
}

void IOBuffer::WriteChar(char val)
{
    m_buffer.Write(val);
}

void IOBuffer::WriteShort(int16_t _val)
{
    auto val = _val;
    flip16(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteUShort(uint16_t _val)
{
    auto val = _val;
    flip16(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteInt(int32_t _val)
{
    auto val = _val;
    flip32(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteUInt(uint32_t _val)
{
    auto val = _val;
    flip32(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteLong(int64_t _val)
{
    auto val = _val;
    flip64(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteULong(uint64_t _val)
{
    auto val = _val;
    flip64(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteFloat(float _val)
{
    auto val = _val;
    flip32(&val);
    m_buffer.Write(val);
}

void IOBuffer::WriteDouble(double _val)
{
    auto val = _val;
    flip64(&val);
    m_buffer.Write(val);
}

void IOBuffer::SetEndian(bool bigEndian)
{
    m_flipEndian = IsBigEndian() != bigEndian;
}

size_t IOBuffer::size() const
{
    return m_buffer.m_writeIdx;
}

// for write
void IOBuffer::jump(size_t size)
{
    m_buffer.EnsureWritable(size);
    m_buffer.m_writeIdx += size;
}

// for read
void IOBuffer::skip(size_t size)
{
    m_buffer.m_readIdx += size;
}

void IOBuffer::read(size_t size, void* dst)
{
    m_buffer.Read(size, (BYTE*)dst);
}

void IOBuffer::write(size_t size, const void* src)
{
    m_buffer.Write(size, (const BYTE*)src);
}

void IOBuffer::flip16(void* val)
{
    if (!m_flipEndian)
        return;

    BYTE* data = (BYTE*)val;
    std::swap(data[0], data[1]);
}

void IOBuffer::flip32(void* val)
{
    if (!m_flipEndian)
        return;

    BYTE* data = (BYTE*)val;
    std::swap(data[0], data[3]);
    std::swap(data[1], data[2]);
}

void IOBuffer::flip64(void* val)
{
    if (!m_flipEndian)
        return;

    BYTE* data = (BYTE*)val;
    std::swap(data[0], data[7]);
    std::swap(data[1], data[6]);
    std::swap(data[2], data[5]);
    std::swap(data[3], data[4]);
}
