#include "Engine/Network/Message.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


extern unsigned short (*HTONS)(unsigned short);
extern unsigned short (*NTOHS)(unsigned short);
extern unsigned long (*HTONL)(unsigned long);
extern unsigned long (*NTOHL)(unsigned long);

Message::Message()
{
}

Message::Message(const Message& copyFrom)
    : m_isEcho(copyFrom.m_isEcho)
    , m_msg(copyFrom.m_msg)
{
}

Message::Message(bool is_echo, const std::string& msg)
    : m_isEcho(is_echo)
    , m_msg(msg)
{
}

Message::~Message()
{
}


bool MessageBuffer::ReadMessage(Message& msg)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	size_t pktSize = sizeof(MSG_HEADER);
	if (!IsReadable(pktSize))
		return false;
	MSG_HEADER header;
	std::copy(m_data.begin(), m_data.begin() + sizeof(MSG_HEADER), (char*) &header);
	header.size = NTOHS(header.size);

	pktSize += header.size;
	if (!IsReadable(pktSize))
		return false;

	uint16_t msg_length;
	char nullchar;

	Read(header);
	header.size = NTOHS(header.size);
	Read(msg.m_isEcho);
	Read(msg_length);
	msg_length = NTOHS(msg_length);
	msg.m_msg.resize(msg_length - 1);
	Read(msg_length - 1, &msg.m_msg[0]);
	Read(nullchar);
	return true;
}

void MessageBuffer::WriteMessage(const Message& msg)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	constexpr size_t MESSAGE_PRESERVED_SIZE = sizeof(msg.m_isEcho) + sizeof(uint16_t) + sizeof(char);
	constexpr size_t MAX_ALLOWED_PAYLOAD_SIZE = 0xFFFFULL - MESSAGE_PRESERVED_SIZE;

	if (msg.m_msg.size() > MAX_ALLOWED_PAYLOAD_SIZE)
	{
		ERROR_RECOVERABLE("payload too big");
		return;
	}

	MSG_HEADER header = {};
	header.size += (uint16_t) MESSAGE_PRESERVED_SIZE;
	header.size += (uint16_t) msg.m_msg.size();

	Write(HTONS(header.size));
	Write(msg.m_isEcho);
	Write(HTONS((uint16_t) (msg.m_msg.size() + 1)));
	Write(msg.m_msg.size() + 1, msg.m_msg.c_str());
}

bool MessageBuffer::IsReadable(size_t size)
{
	return m_data.size() >= size;
}

size_t MessageBuffer::ReadBytes(size_t size, char* data)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	if (size > m_data.size())
		size = m_data.size();

	if (size == 0)
		return 0;

	Read(size, data);
	return size;
}

size_t MessageBuffer::WriteBytes(size_t size, const char* data)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);

	if (size == 0)
		return 0;

	Write(size, data);
	return size;
}


