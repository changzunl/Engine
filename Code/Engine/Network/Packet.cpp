#include "Engine/Network/Packet.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


extern unsigned short (*HTONS)(unsigned short);
extern unsigned short (*NTOHS)(unsigned short);
extern unsigned long (*HTONL)(unsigned long);
extern unsigned long (*NTOHL)(unsigned long);

Packet::Packet()
{
}

Packet::Packet(uint16_t id, const std::vector<char>& buffer)
	: m_id(id)
	, m_buffer(buffer)
{
}

Packet::Packet(uint16_t id, size_t bufferSize, const char* buffer)
    : m_id(id)
    , m_buffer(bufferSize)
{
	std::copy(buffer, &buffer[bufferSize], m_buffer.begin());
}

Packet::Packet(const Packet& copyFrom)
    : m_id(copyFrom.m_id)
    , m_buffer(copyFrom.m_buffer)
{

}

Packet::~Packet()
{
}


bool PacketBuffer::ReadMessage(Packet& packet)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	size_t pktSize = sizeof(PKT_HEADER);
	if (!IsReadable(sizeof(PKT_HEADER)))
		return false;
	PKT_HEADER header;
	std::copy(m_data.begin(), m_data.begin() + sizeof(PKT_HEADER), (char*) &header);
	header.type  = NTOHS(header.type);
	header.flags = NTOHS(header.flags);
	header.size  = NTOHL(header.size);

	pktSize += header.size;
	if (!IsReadable(pktSize))
		return false;

    Read(header);
    header.type = NTOHS(header.type);
    header.flags = NTOHS(header.flags);
    header.size = NTOHL(header.size);

	packet.m_id = header.type;
	packet.m_buffer.resize(header.size);
	ReadBytes(header.size, packet.m_buffer.data());
	return true;
}

void PacketBuffer::WriteMessage(const Packet& packet)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

    if ((packet.m_buffer.size() & 0xFFFFFFFF00000000ULL) != 0)
    {
        ERROR_RECOVERABLE("payload too big");
        return;
    }

	PKT_HEADER header = {};
	header.type = packet.m_id;
	header.size = (int)packet.m_buffer.size();

	Write(HTONS(header.type));
	Write(HTONS(header.flags));
	Write(HTONL(header.size));
	WriteBytes(header.size, packet.m_buffer.data());
}

bool PacketBuffer::IsReadable(size_t size)
{
	return m_data.size() >= size;
}

size_t PacketBuffer::ReadBytes(size_t size, char* data)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	if (size > m_data.size())
		size = m_data.size();

	if (size == 0)
		return 0;

	Read(size, data);
	return size;
}

size_t PacketBuffer::WriteBytes(size_t size, const char* data)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	if (size == 0)
		return 0;

	Write(size, data);
	return size;
}


