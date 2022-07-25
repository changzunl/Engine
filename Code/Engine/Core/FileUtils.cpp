#include "FileUtils.hpp"

#include <fstream>
#include <sstream>
#include <iterator>

#include "Engine/Core/ByteBuffer.hpp"

bool FileExists(const std::string& filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	return file.is_open();
}

int FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& filename)
{
	std::ofstream file(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (file.is_open())
	{
		file.write((const char*)inBuffer.data(), inBuffer.size());
		return (int) inBuffer.size();
	}
	else
	{
		return -1;
	}
}

int FileWriteFromBuffer(ByteBuffer& inBuffer, const std::string& filename)
{
	std::ofstream file(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (file.is_open())
	{
		size_t length = inBuffer.m_writeIdx - inBuffer.m_readIdx;
		file.write((const char*)&inBuffer.data()[inBuffer.m_readIdx], length);
		inBuffer.m_readIdx += length;
		return (int)length;
	}
	else
	{
		return -1;
	}
}

int FileWriteFromString(std::string& inString, const std::string& filename)
{
	std::ofstream file(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (file.is_open())
	{
		file.write(inString.data(), inString.size());
		return (int) inString.size();
	}
	else
	{
		return -1;
	}
}

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	std::ifstream file(filename, std::ios::in |std::ios::binary);
	if (file.is_open())
	{
		file.seekg(0, file.end);
		size_t size = file.tellg();
		file.seekg(0, file.beg);

		outBuffer.reserve(size + 1);
		outBuffer.resize(size);
		file.read((char*)outBuffer.data(), size);
		return (int) size;
	}
	else
	{
		return -1;
	}
}

int FileReadToBuffer(ByteBuffer& outBuffer, const std::string& filename)
{
	std::ifstream file(filename, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		file.seekg(0, file.end);
		size_t size = file.tellg();
		file.seekg(0, file.beg);

		outBuffer.EnsureWritable(size);
		file.read((char*)&outBuffer.data()[outBuffer.m_writeIdx], size);
		outBuffer.m_writeIdx += size;
		return (int)size;
	}
	else
	{
		return -1;
	}
}

int FileReadToString(std::string& outString, const std::string& filename)
{
	std::ifstream file(filename, std::ios::in);
	if (file.is_open())
	{
		file.seekg(0, file.end);
		size_t size = file.tellg();
		file.seekg(0, file.beg);

		std::stringstream buffer;
		buffer << file.rdbuf();

		outString = buffer.str();
		return (int) size;
	}
	else
	{
		return -1;
	}
}

