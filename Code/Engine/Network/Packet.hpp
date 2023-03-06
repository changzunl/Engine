// ================ BUFFER  UTILITIES SECTION =============== //
#include "Engine/Core/ErrorWarningAssert.hpp"

#pragma once

#include <deque>
#include <vector>
#include <string>
#include <mutex>


class Packet
{
public:
	uint16_t m_id = 0;
	std::vector<char> m_buffer;

public:
	Packet();
    Packet(uint16_t id, const std::vector<char>& buffer);
    Packet(uint16_t id, size_t bufferSize, const char* buffer);
	Packet(const Packet& copyFrom);
	~Packet();

};

class PacketBuffer
{
private:
	struct PKT_HEADER
	{
		uint16_t type;
		uint16_t flags;
		uint32_t size;
	};

public:
	std::recursive_mutex m_lock;
	std::deque<char> m_data;

public:
	size_t ReadBytes(size_t size, char* data);
	size_t WriteBytes(size_t size, const char* data);

	bool ReadMessage(Packet& packet);
	void WriteMessage(const Packet& packet);

private:
	template<typename T>
	void Write(size_t arrLen, const T* arrPtr)
	{
		const char* begin = (const char*) arrPtr;
		const char* end   = (const char*) &arrPtr[arrLen];
		m_data.insert(m_data.end(), begin, end);
	}

	template<typename T>
	void Read(size_t arrLen, T* arrPtr)
	{
		char* begin = (char*) arrPtr;
		size_t dataSize = sizeof(T) * arrLen;

		std::copy(m_data.begin(), m_data.begin() + dataSize, begin);
		m_data.erase(m_data.begin(), m_data.begin() + dataSize);
	}

	template<typename T>
	void Write(const T& data)
	{
		Write(1, &data);
	}

	template<typename T>
	void Read(T& data)
	{
		Read(1, &data);
	}

	bool IsReadable(size_t size);
};


