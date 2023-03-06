// ================ BUFFER  UTILITIES SECTION =============== //
#include "Engine/Core/ErrorWarningAssert.hpp"

#pragma once

#include <deque>
#include <string>
#include <mutex>


class Message
{
public:
	uint8_t m_isEcho = 0;
	std::string m_msg;

public:
	Message();
	Message(bool is_echo, const std::string& msg);
	Message(const Message& copyFrom);
	~Message();

};

class MessageBuffer
{
private:
	struct MSG_HEADER
	{
		uint16_t size;
	};

public:
	std::recursive_mutex m_lock;
	std::deque<char> m_data;

public:
	size_t ReadBytes(size_t size, char* data);
	size_t WriteBytes(size_t size, const char* data);

	bool ReadMessage(Message& msg);
	void WriteMessage(const Message& msg);

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


