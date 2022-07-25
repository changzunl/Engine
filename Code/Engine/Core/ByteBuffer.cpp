#include "Engine/Core/ByteBuffer.hpp"


ByteBuffer::ByteBuffer(size_t initialCapacity /*= 0*/) : m_data(initialCapacity)
{

}

ByteBuffer::ByteBuffer(const ByteBuffer& copyFrom, size_t offset /*= 0*/, size_t length /*= -1*/)
{
	if (length == -1)
		length = copyFrom.m_writeIdx - copyFrom.m_readIdx + offset;

	m_data.resize(length);
	memcpy(m_data.data(), &copyFrom.m_data[copyFrom.m_readIdx + offset], length);
}

ByteBuffer::~ByteBuffer()
{
}

size_t ByteBuffer::WriteZero(size_t size)
{
	if (m_data.size() < m_writeIdx + size)
		m_data.resize(m_writeIdx + size);

	memset(&m_data[m_writeIdx], 0, size);
	m_writeIdx += size;
	return size;
}

size_t ByteBuffer::Skip(size_t size)
{
	if (m_readIdx + size > m_writeIdx)
		size = m_writeIdx - m_readIdx;

	m_readIdx += size;
	return size;
}

size_t ByteBuffer::WriteAlignment()
{
	size_t size = 0;
	BYTE b = 0;
	while (m_writeIdx % 4 != 0)
		size += Write(b);
	return size;
}

size_t ByteBuffer::ReadAlignment()
{
	size_t size = 0;
	BYTE b;
	while (m_readIdx % 4 != 0)
		size += Read(b);
	return size;
}

BYTE* ByteBuffer::data()
{
	return m_data.data();
}

void ByteBuffer::Reset()
{
	m_readIdx = m_writeIdx = 0;
}

void ByteBuffer::ResetRead()
{
	m_readIdx = 0;
}

void ByteBuffer::ShrinkBuffer()
{
	size_t length = m_writeIdx - m_readIdx;
	memmove(data(), &data()[m_readIdx], length);
	m_readIdx = 0;
	m_writeIdx = length;
}

bool ByteBuffer::IsReadable(size_t size)
{
	return (m_readIdx + size) <= m_writeIdx;
}

void ByteBuffer::EnsureWritable(size_t size)
{
	if (m_data.size() < m_writeIdx + size)
		m_data.resize(m_writeIdx + size);
}

bool ByteUtils::IsPlatformBigEndian()
{
	uint32_t i = 0x01020304;
	char* charArr = (char*)&i;
	return charArr[0] == 0x01; // test if first byte is MSB
}

void ByteUtils::ReverseBytes(int16_t& val)
{
	int16_t reversed = 0;
	reversed |= (val >> 0) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 8) & 0xFF;
	val = reversed;
}

void ByteUtils::ReverseBytes(int32_t& val)
{
	int32_t reversed = 0;
	reversed |= (val >>  0) & 0xFF;
	reversed <<= 8;
	reversed |= (val >>  8) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 16) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 24) & 0xFF;
	val = reversed;
}

void ByteUtils::ReverseBytes(int64_t& val)
{
	int64_t reversed = 0;
	reversed |= (val >>  0) & 0xFF;
	reversed <<= 8;
	reversed |= (val >>  8) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 16) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 24) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 32) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 40) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 48) & 0xFF;
	reversed <<= 8;
	reversed |= (val >> 56) & 0xFF;
	val = reversed;
}

void ByteUtils::ReverseBytes16(BYTE* ptr)
{
	constexpr int width = 2;
	BYTE ch[width];
	for (int i = 0; i < width; i++)
		ch[i] = ptr[(width - 1) - i];

	for (int i = 0; i < width; i++)
		ptr[i] = ch[i];
}

void ByteUtils::ReverseBytes32(BYTE* ptr)
{
	constexpr int width = 4;
	BYTE ch[width];
	for (int i = 0; i < width; i++)
		ch[i] = ptr[(width - 1) - i];

	for (int i = 0; i < width; i++)
		ptr[i] = ch[i];
}

void ByteUtils::ReverseBytes64(BYTE* ptr)
{
	constexpr int width = 8;
	BYTE ch[width];
	for (int i = 0; i < width; i++)
		ch[i] = ptr[(width - 1) - i];

	for (int i = 0; i < width; i++)
		ptr[i] = ch[i];
}

void ByteUtils::ReadString(ByteBuffer* buffer, std::string& val)
{
	buffer->ReadAlignment();
	size_t size;
	buffer->Read(size);
	val.resize(size);
	buffer->Read(size, &val[0]);

	buffer->ReadAlignment();
}

void ByteUtils::WriteString(ByteBuffer* buffer, const std::string& val)
{
	buffer->WriteAlignment();
	buffer->Write(val.size());
	buffer->Write(val.size(), val.data());

	buffer->WriteAlignment();
}

void ByteUtils::WriteString(ByteBuffer* buffer, const char* data)
{
	WriteString(buffer, std::string(data));
}

void ByteUtils::ReadString_Net(ByteBuffer* buffer, std::string& val)
{
	buffer->ReadAlignment();
	size_t size;
	buffer->Read(size);
	if (!IsPlatformBigEndian())
		ReverseBytes((int64_t&)size);
	val.resize(size);
	buffer->Read(size, &val[0]);

	buffer->ReadAlignment();
}

void ByteUtils::WriteString_Net(ByteBuffer* buffer, const std::string& val)
{
	buffer->WriteAlignment();
	size_t size = val.size();
	if (!IsPlatformBigEndian())
		ReverseBytes((int64_t&)size);
	buffer->Write(size);
	buffer->Write(val.size(), val.data());

	buffer->WriteAlignment();
}

void ByteUtils::WriteString_Net(ByteBuffer* buffer, const char* data)
{
	WriteString_Net(buffer, std::string(data));
}

void ByteUtils::ReadVarInt(ByteBuffer* buffer, int& val)
{
	constexpr unsigned char mask = 1 << 7;
	val = 0;

	unsigned char b;
	do
	{
		buffer->Read(b);
		val <<= 7;
		val |= b & ~mask;
	} while ((b & mask) == mask);
}

void ByteUtils::WriteVarInt(ByteBuffer* buffer, int val)
{
	constexpr unsigned char mask = 1 << 7;

	unsigned char b;
	do
	{
		b = (unsigned char)val & ~mask;
		val >>= 7;
		if (val != 0)
			b |= mask;
		buffer->Write(b);
	} while ((b & mask) == mask);
}

NetworkBuffer::NetworkBuffer(ByteBuffer* buffer)
	: m_buffer(buffer)
	, m_transEndian(!ByteUtils::IsPlatformBigEndian())
{
}

BYTE NetworkBuffer::ReadByte()
{
	BYTE b;
	m_buffer->Read(b);
	return b;
}

int32_t NetworkBuffer::ReadInt()
{
	int32_t i;
	m_buffer->Read(i);
	if (m_transEndian)
		ByteUtils::ReverseBytes(i);
	return i;
}

uint32_t NetworkBuffer::ReadUInt()
{
	int32_t i = ReadInt();
	return (uint32_t&)i;
}

float NetworkBuffer::ReadFloat()
{
	int32_t i = ReadInt();
	return (float&)i;
}

double NetworkBuffer::ReadDouble()
{
	int64_t i = ReadLong();
	return (double&)i;
}

size_t NetworkBuffer::ReadSize32()
{
	// assert(sizeof(size_t) == 4);

	int32_t i = ReadInt();
	return (size_t&)i;
}

size_t NetworkBuffer::ReadSize64()
{
	// assert(sizeof(size_t) == 8);

	int64_t i = ReadLong();
	return (size_t&)i;
}

void NetworkBuffer::WriteByte(BYTE val)
{
	m_buffer->Write(val);
}

void NetworkBuffer::WriteInt(int32_t val)
{
	ByteUtils::ReverseBytes(val);
	m_buffer->Write(val);
}

void NetworkBuffer::WriteUInt(uint32_t val)
{
	WriteInt((int32_t&)val);
}

void NetworkBuffer::WriteFloat(float val)
{
	WriteInt((int32_t&)val);
}

void NetworkBuffer::WriteDouble(double val)
{
	WriteLong((int64_t&)val);
}

void NetworkBuffer::WriteLong(int64_t val)
{
	if (m_transEndian)
		ByteUtils::ReverseBytes(val);
	m_buffer->Write(val);
}

void NetworkBuffer::WriteULong(uint64_t val)
{
	WriteLong((int64_t&)val);
}

void NetworkBuffer::WriteSize32(size_t val)
{
	WriteInt((int32_t&)val);
}

void NetworkBuffer::WriteSize64(size_t val)
{
	WriteLong((int64_t&)val);
}

std::string&& NetworkBuffer::ReadString()
{
	std::string str;
	size_t size = ReadSize64();
	str.resize(size);
	m_buffer->Read(size, &str[0]);
	return std::move(str);
}

void NetworkBuffer::WriteString(const std::string& val)
{
	WriteSize64(val.size());
	m_buffer->Write(val.size(), val.data());
}

void NetworkBuffer::WriteString(const char* data)
{
	WriteString(std::string(data));
}

int64_t NetworkBuffer::ReadLong()
{
	int64_t i;
	m_buffer->Read(i);
	if (m_transEndian)
		ByteUtils::ReverseBytes(i);
	return i;
}

uint64_t NetworkBuffer::ReadULong()
{
	int64_t i = ReadLong();
	return (uint64_t&)i;
}
