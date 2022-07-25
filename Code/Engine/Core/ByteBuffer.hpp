// ================ BUFFER  UTILITIES SECTION =============== //
#pragma once

#include <vector>
#include <string>

typedef unsigned char BYTE;

constexpr char DATA_TYPE_FLOAT       = 1; // floating point number
constexpr char DATA_TYPE_INTEGER     = 0; // integer number

enum class DataType
{
	FLOAT       = (DATA_TYPE_FLOAT    << 16) | 4,
	DOUBLE      = (DATA_TYPE_FLOAT    << 16) | 8,
	INT         = (DATA_TYPE_INTEGER  << 16) | 4,
	SHORT       = (DATA_TYPE_INTEGER  << 16) | 2,
	LONG        = (DATA_TYPE_INTEGER  << 16) | 8,
};

class ByteBuffer
{
private:
	std::vector<BYTE> m_data;

public:
	size_t m_readIdx = 0;
	size_t m_writeIdx = 0;

public:
	ByteBuffer(size_t initialCapacity = 0);
	ByteBuffer(const ByteBuffer& copyFrom, size_t offset = 0, size_t length = -1);
	virtual ~ByteBuffer();
	
	template<typename T>
	size_t Write(size_t arrLen, const T* arrPtr)
	{
		if (!arrLen)
			return 0;

		size_t appendSize = sizeof(T) * arrLen;

		if (m_data.size() < m_writeIdx + appendSize)
			m_data.resize(m_writeIdx + appendSize);

		memcpy(&m_data[m_writeIdx], arrPtr, appendSize);
		m_writeIdx += appendSize;
		return appendSize;
	}

	template<typename T>
	size_t Write(const T& val)
	{
		return Write(1, &val);
	}

	template<typename T>
	size_t Read(size_t arrLen, T* arrPtr)
	{
		if (!arrLen)
			return 0;

		size_t shrinkSize = sizeof(T) * arrLen;

		if (m_readIdx + shrinkSize > m_writeIdx)
			shrinkSize = m_writeIdx - m_readIdx;
		
		memcpy(arrPtr, &m_data[m_readIdx], shrinkSize);
		m_readIdx += shrinkSize;
		return shrinkSize;
	}

	template<typename T>
	size_t Read(T& val)
	{
		return Read(1, &val);
	}

	template<typename T>
	size_t WriteObject(size_t arrLen, const T* arrPtr)
	{
		size_t prevWriteIdx = m_writeIdx;
		for (size_t i = 0; i < arrLen; i++)
			WriteObject(arrPtr[i]);
		return m_writeIdx - prevWriteIdx;
	}

	template<typename T>
	size_t WriteObject(size_t arrLen, const T** arrPtr)
	{
		size_t prevWriteIdx = m_writeIdx;
		for (size_t i = 0; i < arrLen; i++)
			WriteObject(*arrPtr[i]);
		return m_writeIdx - prevWriteIdx;
	}

	template<typename T>
	size_t WriteObject(const T& val)
	{
		size_t prevWriteIdx = m_writeIdx;
		val.WriteBytes(this);
		return m_writeIdx - prevWriteIdx;
	}

	template<typename T>
	size_t ReadObject(size_t arrLen, T* arrPtr)
	{
		size_t prevReadIdx = m_readIdx;
		for (size_t i = 0; i < arrLen; i++)
			ReadObject(arrPtr[i]);
		return m_readIdx - prevReadIdx;
	}

	template<typename T>
	size_t ReadObject(size_t arrLen, T** arrPtr)
	{
		size_t prevReadIdx = m_readIdx;
		for (size_t i = 0; i < arrLen; i++)
		{
			T* ptr = arrPtr[i];
			if (!ptr)
				ptr = arrPtr[i] = new T();
			ReadObject(*ptr);
		}
		return m_readIdx - prevReadIdx;
	}

	template<typename T>
	size_t ReadObject(T& val)
	{
		size_t prevReadIdx = m_readIdx;
		val.ReadBytes(this);
		return m_readIdx - prevReadIdx;
	}

	size_t WriteZero(size_t size);

	size_t Skip(size_t size);

	size_t WriteAlignment();
	size_t ReadAlignment();

	BYTE* data();

	void Reset();

	void ResetRead();

	void ShrinkBuffer();

	bool IsReadable(size_t size);

	void EnsureWritable(size_t size);
};

class ByteUtils
{
public:
	static bool IsPlatformBigEndian();

	static void ReverseBytes(int16_t& val);

	static void ReverseBytes(int32_t& val);

	static void ReverseBytes(int64_t& val);

	static void ReverseBytes16(BYTE* ptr);

	static void ReverseBytes32(BYTE* ptr);

	static void ReverseBytes64(BYTE* ptr);

	static void ReadString(ByteBuffer* buffer, std::string& val);

	static void WriteString(ByteBuffer* buffer, const std::string& val);

	static void WriteString(ByteBuffer* buffer, const char* data);

	static void ReadString_Net(ByteBuffer* buffer, std::string& val);

	static void WriteString_Net(ByteBuffer* buffer, const std::string& val);

	static void WriteString_Net(ByteBuffer* buffer, const char* data);

	static void ReadVarInt(ByteBuffer* buffer, int& val);

	static void WriteVarInt(ByteBuffer* buffer, int val);

	template<typename T>
	static void ReadArray(ByteBuffer* buffer, std::vector<T>& val)
	{
		size_t size;
		buffer->Read(size);
		val.resize(size);
		buffer->Read(size, val.data());
	}

	template<typename T>
	static void WriteArray(ByteBuffer* buffer, const std::vector<T>& val)
	{
		buffer->Write(val.size());
		buffer->Write(val.size(), val.data());
	}

	template<typename T>
	static void ReadObjects(ByteBuffer* buffer, std::vector<T>& val)
	{
		size_t size;
		buffer->Read(size);
		val.resize(size);
		for (T& obj : val)
			obj.ReadBytes(buffer);
	}

	template<typename T>
	static void WriteObjects(ByteBuffer* buffer, const std::vector<T>& val)
	{
		buffer->Write(val.size());
		for (const T& obj : val)
			obj.WriteBytes(buffer);
	}

	template<typename T>
	static void ReadArray_Net(ByteBuffer* buffer, std::vector<T>& val, DataType type)
	{
		if (IsPlatformBigEndian())
		{
			size_t size;
			buffer->Read(size);
			val.resize(size);
			buffer->Read(size, val.data());
		}
		else
		{
			size_t size;
			buffer->Read(size);
			ReverseBytes((int64_t&)size);
			val.resize(size);

			size_t readIdx = buffer->m_readIdx;
			buffer->Read(size, val.data());

			BYTE* data = (BYTE*)val.data();

			if (((int)type | 0xFFFF) == 2)
				for (size_t idx = readIdx; idx < buffer->m_readIdx; idx += 2)
					ByteUtils::ReverseBytes16(&data[idx]);
			else if (((int)type | 0xFFFF) == 4)
				for (size_t idx = readIdx; idx < buffer->m_readIdx; idx += 4)
					ByteUtils::ReverseBytes32(&data[idx]);
			else if (((int)type | 0xFFFF) == 8)
				for (size_t idx = readIdx; idx < buffer->m_readIdx; idx += 8)
					ByteUtils::ReverseBytes64(&data[idx]);
		}
	}

	template<typename T>
	static void WriteArray_Net(ByteBuffer* buffer, const std::vector<T>& val, DataType type)
	{
		if (IsPlatformBigEndian())
		{
			buffer->Write(val.size());
			buffer->Write(val.size(), val.data());
		}
		else
		{
			size_t size = val.size();
			ReverseBytes((int64_t&)size);

			buffer->Write(size);

			size_t writeIdx = buffer->m_writeIdx;
			buffer->Write(val.size(), val.data());

			if      (((int)type | 0xFFFF) == 2)
				for (size_t idx = writeIdx; idx < buffer->m_writeIdx; idx += 2)
					ByteUtils::ReverseBytes16(&buffer->data()[idx]);
			else if (((int)type | 0xFFFF) == 4)
				for (size_t idx = writeIdx; idx < buffer->m_writeIdx; idx += 4)
					ByteUtils::ReverseBytes32(&buffer->data()[idx]);
			else if (((int)type | 0xFFFF) == 8)
				for (size_t idx = writeIdx; idx < buffer->m_writeIdx; idx += 8)
					ByteUtils::ReverseBytes64(&buffer->data()[idx]);
		}
	}
};

class NetworkBuffer
{
public:
	NetworkBuffer(ByteBuffer* buffer);

	BYTE      ReadByte();
	int32_t   ReadInt();
	uint32_t  ReadUInt();
	float     ReadFloat();
	double    ReadDouble();
	int64_t   ReadLong();
	uint64_t  ReadULong();
	size_t    ReadSize32();
	size_t    ReadSize64();

	void WriteByte(BYTE val);
	void WriteInt(int32_t val);
	void WriteUInt(uint32_t val);
	void WriteFloat(float val);
	void WriteDouble(double val);
	void WriteLong(int64_t val);
	void WriteULong(uint64_t val);
	void WriteSize32(size_t val);
	void WriteSize64(size_t val);

	std::string&& ReadString();
	void WriteString(const std::string& val);
	void WriteString(const char* data);

private:
	ByteBuffer* m_buffer;
	bool        m_transEndian;
};

