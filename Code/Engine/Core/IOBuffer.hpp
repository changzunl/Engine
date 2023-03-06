#pragma once

#include "Engine/Core/ByteBuffer.hpp"
#include <string>


class IOBuffer
{
public:
	IOBuffer(size_t initialCapacity = 0);

	BYTE              ReadByte();
	char              ReadChar();
	int16_t           ReadShort();
	uint16_t          ReadUShort();
	int32_t           ReadInt();
	uint32_t          ReadUInt();
	int64_t           ReadLong();
	uint64_t          ReadULong();
	float             ReadFloat();
	double            ReadDouble();
	size_t            ReadSize32();
	size_t            ReadSize64();

	void WriteByte(BYTE val);
	void WriteChar(char val);
	void WriteShort(int16_t val);
	void WriteUShort(uint16_t val);
	void WriteInt(int32_t val);
	void WriteUInt(uint32_t val);
	void WriteLong(int64_t val);
	void WriteULong(uint64_t val);
	void WriteFloat(float val);
	void WriteDouble(double val);

	void SetEndian(bool bigEndian);

	size_t size() const;
	void jump(size_t size); // for write
	void skip(size_t size); // for read
	void read(size_t size, void* dst);
	void write(size_t size, const void* src);

private:
	void flip16(void* val);
	void flip32(void* val);
	void flip64(void* val);

public:
	ByteBuffer m_buffer;

private:
	bool m_flipEndian;
};
