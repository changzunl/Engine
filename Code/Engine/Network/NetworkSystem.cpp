#include "Engine/Network/NetworkSystem.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <WinSock2.h>
#include <Ws2Tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


WSADATA NET_WINSOCK_INFO = {};
bool NET_WINSOCK_INITIALIZED = false;
constexpr int NET_READ_BUFFER_SIZE = 8192;
constexpr int NET_WRITE_BUFFER_SIZE = 8192;

unsigned short (*HTONS)(unsigned short) = nullptr;
unsigned short (*NTOHS)(unsigned short) = nullptr;
unsigned long (*HTONL)(unsigned long) = nullptr;
unsigned long (*NTOHL)(unsigned long) = nullptr;

NetErrResult StartupNetworking()
{
	NetErrResult result;

	if (NET_WINSOCK_INITIALIZED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Already running!";
		return result;
	}


	result.m_errCode = ::WSAStartup(MAKEWORD(2, 2), &NET_WINSOCK_INFO);
	if (result.m_errCode != 0)
	{
		result.m_errMsg = "WSAStartup failed";
		return result;
	}

	HTONS = htons;
	NTOHS = ntohs;
	HTONL = htonl;
	NTOHL = ntohl;

	result.m_errMsg = "success!";
	NET_WINSOCK_INITIALIZED = true;
	return result;
}


NetErrResult StopNetworking()
{
	NetErrResult result;

	if (!NET_WINSOCK_INITIALIZED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Not running!";
		return result;
	}

	::WSACleanup();
	NET_WINSOCK_INFO = {};
	NET_WINSOCK_INITIALIZED = false;
	return result;
}

NetErrResult QueryState(SOCKET socket, bool write, bool& state)
{
	NetErrResult result;
	state = false;

	fd_set fd_sets = {};
	fd_sets.fd_count = 1;
	fd_sets.fd_array[0] = socket;
	timeval time = { 0L, 2000L };

	int selCount = ::select(0, write ? nullptr : &fd_sets, write ? &fd_sets : nullptr, nullptr, &time);
	if (selCount == SOCKET_ERROR) // error
	{
		result.m_errCode = ::WSAGetLastError();
		result.m_errMsg = "select failed";
	}

	if (selCount != 1 && selCount != 0)
	{
		result.m_errCode = -1;
		result.m_errMsg = Stringf("unknown error(%d)", selCount);
	}

	state = selCount != 0;
	return result;
}

const char* GetNameFromType(CONNECTION_STATE type)
{
    static const char* const names[4] = { "DISCONNECTED", "ESTABLISHING", "CONNECTED", "LISTENING" };
    return names[(unsigned int)type];
}

CONNECTION_STATE GetTypeByName(const char* name, CONNECTION_STATE defaultType)
{
    static const CONNECTION_STATE types[4] = { CONNECTION_STATE::CONNECTED, CONNECTION_STATE::ESTABLISHING, CONNECTION_STATE::CONNECTED, CONNECTION_STATE::LISTENING };
    for (CONNECTION_STATE type : types)
    {
        if (_stricmp(GetNameFromType(type), name) == 0)
            return type;
    }
    return defaultType;
}

struct AddrInfoPtr
{
	~AddrInfoPtr()
	{
		if (pAddrInfo)
			::freeaddrinfo(pAddrInfo);
	}

	addrinfo* pAddrInfo = nullptr;
};

NetErrResult NetworkManagerClient::CreateClient(const char* host, const char* port, int retryTMillis, int maxRetry)
{
	NetErrResult result;
	if (m_running)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Already connected!";
		return result;
	}

	std::string strHost = host;
	std::string strPort = port;

	m_endpointAddr = Stringf("%s:%s", host, port);
	m_thread = new std::thread([this, strHost, strPort, retryTMillis, maxRetry]() { RunNetworkThread(strHost.c_str(), strPort.c_str(), retryTMillis, maxRetry); });
	return result;
}

void NetworkManagerClient::ReleaseClient()
{
	if (!m_running)
		return;

	m_disconnect = 1;
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;
}

void NetworkManagerClient::NetworkTickClient()
{
	Message packet;
	while (m_bufferRecv.ReadMessage(packet))
		HandleMessage(packet);

	for (const auto& pkt : m_queue)
		m_bufferSend.WriteMessage(pkt);
	m_queue.clear();
}

void NetworkManagerClient::SendToServer(Message& pkt)
{
	m_queue.push_back(pkt);
}

void NetworkManagerClient::HandleMessage(Message& pkt)
{
	auto ite = m_handlers.find(0);
	if (ite != m_handlers.end())
		(*ite).second(pkt);
}

void NetworkManagerClient::RegisterHandler(MessageHandler handler)
{
	m_handlers[0] = handler;
}

void NetworkManagerClient::RunNetworkThread(const char* host, const char* port, int retryTimeMillis, int maxRetry)
{
	m_running = true;

	NetErrResult result;

	maxRetry++;
	while (maxRetry > 0 && m_disconnect == 0)
	{
		DebuggerPrintf("[NETWORK] Connecting to %s:%s...\n", host, port);

		{
			const std::lock_guard<std::recursive_mutex> lockSend(m_bufferSend.m_lock);
			const std::lock_guard<std::recursive_mutex> lockRecv(m_bufferRecv.m_lock);
			m_bufferSend.m_data.clear();
			m_bufferRecv.m_data.clear();
		}

		Connect(host, port, true);

		if (m_state == CONNECTION_STATE::ESTABLISHING)
			DebuggerPrintf("[NETWORK] Establishing connection...\n");
		while (m_state == CONNECTION_STATE::ESTABLISHING)
		{
			ValidateConnection();

			if (m_disconnect)
			{
				m_disconnect = 0;
				CloseConnection();
				DebuggerPrintf("[NETWORK] Disconnected!\n");
				goto OUTTER_LOOP_BREAK;
			}
			std::this_thread::yield();
		}

		if (m_state == CONNECTION_STATE::CONNECTED)
			DebuggerPrintf("[NETWORK] Connected!\n");

		while (m_state == CONNECTION_STATE::CONNECTED)
		{
			ClientSendData();
			ClientRecvData();

			if (m_disconnect)
			{
				m_disconnect = 0;
				CloseConnection();
				DebuggerPrintf("[NETWORK] Disconnected!\n");
				goto OUTTER_LOOP_BREAK;
			}
			std::this_thread::yield();
		}


		if (m_disconnect)
		{
			m_disconnect = 0;
			CloseConnection();
			DebuggerPrintf("[NETWORK] Disconnected!\n");
			goto OUTTER_LOOP_BREAK;
		}
		DebuggerPrintf("[NETWORK] Server disconnected!\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(retryTimeMillis));
		maxRetry--;
		DebuggerPrintf("[NETWORK] Retry count left: %d\n", maxRetry);
	}
	
OUTTER_LOOP_BREAK:
	m_running = false;
}

NetErrResult NetworkManagerClient::Connect(const char* host, const char* port, bool nio)
{
	NetErrResult result = {};

	if (m_state != CONNECTION_STATE::DISCONNECTED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Already connected";
		return result;
	}

	if (!NET_WINSOCK_INITIALIZED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Winsock not initialized!";
		return result;
	}

	AddrInfoPtr pAIResult;
	addrinfo  addrinfoHints = {};

	addrinfoHints.ai_family = AF_UNSPEC;
	addrinfoHints.ai_socktype = SOCK_STREAM;
	addrinfoHints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	result.m_errCode = ::getaddrinfo(host, port, &addrinfoHints, &pAIResult.pAddrInfo);
	if (result.m_errCode != 0) {
		result.m_errMsg = "getaddrinfo failed";
		return result;
	}

	// Attempt to connect to the first address returned by the call to getaddrinfo

	// Create a SOCKET for connecting to server
	m_socket = ::socket(pAIResult.pAddrInfo->ai_family, pAIResult.pAddrInfo->ai_socktype, pAIResult.pAddrInfo->ai_protocol);

	if (m_socket == INVALID_SOCKET) {
		result.m_errMsg = Stringf("Error at socket(): %ld", ::WSAGetLastError());
		return result;
	}

	if (nio)
	{
		u_long mode = 1; // non-blocking
		result.m_errCode = ::ioctlsocket(m_socket, FIONBIO, &mode);
		if (result.m_errCode != 0)
		{
			result.m_errMsg = Stringf("ioctlsocket failed: %d", ::WSAGetLastError());
			return result;
		}
	}

	// Connect to server.
	m_state = CONNECTION_STATE::ESTABLISHING;
	result.m_errCode = ::connect(m_socket, pAIResult.pAddrInfo->ai_addr, (int)pAIResult.pAddrInfo->ai_addrlen);
	if (result.m_errCode == SOCKET_ERROR) {
		result.m_errCode = ::WSAGetLastError();
		if (result.m_errCode != WSAEWOULDBLOCK)
		{
			CloseConnection();
			result.m_errMsg = Stringf("Unable to connect to server!");
			return result;
		}

		result.m_errMsg = "establishing!";
		return result;
	}

	sockaddr_in* ipv4 = (sockaddr_in*)pAIResult.pAddrInfo->ai_addr;
	m_endpointAddr = Stringf("%d.%d.%d.%d:%d", ipv4->sin_addr.S_un.S_un_b.s_b1, ipv4->sin_addr.S_un.S_un_b.s_b2, ipv4->sin_addr.S_un.S_un_b.s_b3, ipv4->sin_addr.S_un.S_un_b.s_b4, ipv4->sin_port);
	m_state = CONNECTION_STATE::CONNECTED;
	result.m_errMsg = "success!";
	return result;
}

NetErrResult NetworkManagerClient::ClientRecvData()
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::CONNECTED)
	{
		result.m_errCode = 1;
		result.m_errMsg = "Not connected";
		return result;
	}

	bool state = false;
	result = QueryState(m_socket, false, state);
	if (result.m_errCode != 0)
	{
		CloseConnection();
		return result;
	}
	if (!state)
	{
		result.m_errCode = 0;
		result.m_errMsg = "blocking";
		return result;
	}

	char buffer[NET_READ_BUFFER_SIZE] = {};

	int lengthRecv = ::recv(m_socket, buffer, NET_READ_BUFFER_SIZE, 0);
	if (lengthRecv == SOCKET_ERROR)
	{
		result.m_errCode = ::WSAGetLastError();
		if (result.m_errCode != WSAEWOULDBLOCK)
		{
			result.m_errMsg = "Socket error";
			CloseConnection();
			return result;
		}

		// blocking, do nothing
		result.m_errCode = 0;
		return result;
	}
	else if (lengthRecv == 0)
	{
		result.m_errCode = WSAECONNRESET;
		result.m_errMsg = "Connection reset";
		CloseConnection();
		return result;
	}

	DebuggerPrintf("Recv data %d bytes\n", lengthRecv);

	m_bufferRecv.WriteBytes(lengthRecv, buffer);

	return result;
}

NetErrResult NetworkManagerClient::ClientSendData()
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::CONNECTED)
	{
		result.m_errCode = 1;
		result.m_errMsg = "Not connected";
		return result;
	}

	bool state = false;
	result = QueryState(m_socket, true, state);
	if (result.m_errCode != 0)
	{
		CloseConnection();
		return result;
	}
	if (!state)
	{
		result.m_errCode = 0;
		result.m_errMsg = "blocking";
		return result;
	}

	char buffer[NET_WRITE_BUFFER_SIZE] = {};
	int length = (int)m_bufferSend.ReadBytes(NET_WRITE_BUFFER_SIZE, buffer);

	if (length == 0)
	{
		result.m_errMsg = "empty";
		return result;
	}

	int lengthSent = ::send(m_socket, buffer, length, 0);
	if (lengthSent == SOCKET_ERROR)
	{
		result.m_errCode = ::WSAGetLastError();
		if (result.m_errCode != WSAEWOULDBLOCK)
		{
			result.m_errMsg = "Socket error";
			CloseConnection();
			return result;
		}

		// blocking, do nothing
		result.m_errCode = 0;
		return result;
	}

	DebuggerPrintf(Stringf("Sent data %d bytes\n", lengthSent).c_str());

	return result;
}

NetErrResult NetworkManagerClient::ValidateConnection()
{
	NetErrResult result;

	if (m_socket == INVALID_SOCKET)
	{
		result.m_errCode = 1;
		result.m_errMsg = "Invalid socket";
		return result;
	}

	if (m_state != CONNECTION_STATE::ESTABLISHING)
	{
		result.m_errCode = 1;
		result.m_errMsg = "Invalid state";
		return result;
	}

	bool state = false;
	result = QueryState(m_socket, true, state);
	if (result.m_errCode != 0)
	{
		CloseConnection();
		return result;
	}

	if (state)
		m_state = CONNECTION_STATE::CONNECTED;
	return result;
}

NetErrResult NetworkManagerClient::CloseConnection()
{
	NetErrResult result;

	if (m_socket == INVALID_SOCKET)
	{
		result.m_errCode = 1;
		result.m_errMsg = "Invalid socket";
		return result;
	}

	result.m_errCode = ::closesocket(m_socket);
	m_socket = INVALID_SOCKET;
	m_state = CONNECTION_STATE::DISCONNECTED;
	return result;
}

NetworkManagerClient::NetworkManagerClient()
{
}

NetworkManagerClient::~NetworkManagerClient()
{
	ReleaseClient();
}

NetworkManagerServer::ClientConnection::ClientConnection()
{
}

NetworkManagerServer::ClientConnection::~ClientConnection()
{
	ReleaseConnection();
}

void NetworkManagerServer::ClientConnection::CreateConnection()
{
	m_thread = new std::thread([this]() { RunNetworkThread(); });
}

void NetworkManagerServer::ClientConnection::ReleaseConnection()
{
	if (!m_running)
		return;

	m_disconnect = 1;
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;
}

void NetworkManagerServer::ClientConnection::NetworkTickConnection(NetworkManagerServer* server)
{
	Message packet;
	while (m_bufferRecv.ReadMessage(packet))
		server->HandleMessage(this, packet);

	for (const auto& pkt : m_queue)
		m_bufferSend.WriteMessage(pkt);
	m_queue.clear();
}

const std::string& NetworkManagerServer::ClientConnection::GetEndpointAddr() const
{
	return m_endpointAddr;
}

void NetworkManagerServer::ClientConnection::Disconnect()
{
	ReleaseConnection();
}

void NetworkManagerServer::ClientConnection::Send(Message& pkt)
{
	m_queue.push_back(pkt);
}

void NetworkManagerServer::ClientConnection::RunNetworkThread()
{
	m_running = true;
	m_state = CONNECTION_STATE::CONNECTED;

	while (m_state == CONNECTION_STATE::CONNECTED)
	{
		ClientRecvData();
		ClientSendData();

		if (m_disconnect)
		{
			m_disconnect = false;
			CloseConnection();
			break;
		}
	}

	m_running = false;
}

NetErrResult NetworkManagerServer::ClientConnection::ClientRecvData()
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::CONNECTED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Not connected!";
		return result;
	}

	bool state = false;
	result = QueryState(m_socket, false, state);
	if (result.m_errCode != 0)
	{
		CloseConnection();
		return result;
	}
	if (!state)
	{
		result.m_errCode = 0;
		result.m_errMsg = "blocking";
		return result;
	}

	char buffer[NET_READ_BUFFER_SIZE] = {};

	int lengthRecv = ::recv(m_socket, buffer, NET_READ_BUFFER_SIZE, 0);
	if (lengthRecv == SOCKET_ERROR)
	{
		result.m_errCode = ::WSAGetLastError();
		if (result.m_errCode != WSAEWOULDBLOCK)
		{
			result.m_errMsg = "Socket error";
			CloseConnection();
			return result;
		}

		// blocking, do nothing
		result.m_errCode = 0;
		return result;
	}
	else if (lengthRecv == 0)
    {
        result.m_errCode = WSAECONNRESET;
        result.m_errMsg = "Connection reset";
		CloseConnection();
        return result;
	}

	m_bufferRecv.WriteBytes(lengthRecv, buffer);
	return result;
}

NetErrResult NetworkManagerServer::ClientConnection::ClientSendData()
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::CONNECTED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Not connected!";
		return result;
	}

	bool state = false;
	result = QueryState(m_socket, true, state);
	if (result.m_errCode != 0)
	{
		CloseConnection();
		return result;
	}
	if (!state)
	{
		result.m_errCode = 0;
		result.m_errMsg = "blocking";
		return result;
	}

	char buffer[NET_WRITE_BUFFER_SIZE] = {};
	int length = (int)m_bufferSend.ReadBytes(NET_WRITE_BUFFER_SIZE, buffer);

	int lengthSent = ::send(m_socket, buffer, length, 0);
	if (lengthSent == SOCKET_ERROR)
	{
		result.m_errCode = ::WSAGetLastError();
		if (result.m_errCode != WSAEWOULDBLOCK)
		{
			result.m_errMsg = "Socket error";
			CloseConnection();
			return result;
		}

		// blocking, do nothing
		result.m_errCode = 0;
		return result;
	}

	return result;
}

NetErrResult NetworkManagerServer::ClientConnection::CloseConnection()
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::CONNECTED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Not connected!";
		return result;
	}

	result.m_errCode = ::shutdown(m_socket, SD_SEND);
	if (result.m_errCode == SOCKET_ERROR)
	{
		result.m_errCode = ::WSAGetLastError();
		result.m_errMsg = " Failed to shutdown connection";
		::closesocket(m_socket);
		m_state = CONNECTION_STATE::DISCONNECTED;
		return result;
	}

	m_state = CONNECTION_STATE::DISCONNECTED;
	return result;
}

NetworkManagerServer::NetworkManagerServer()
{
}

NetworkManagerServer::~NetworkManagerServer()
{
	ReleaseServer();
}

NetErrResult NetworkManagerServer::CreateServer(const char* host /*= DEFAULT_HOST*/, const char* port /*= DEFAULT_PORT*/)
{
	NetErrResult result;
	if (m_running)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Already running!";
		return result;
	}

	std::string strHost = host;
	std::string strPort = port;

	m_endpointAddr = host;
	m_endpointPort = port;
	m_thread = new std::thread([this, strHost, strPort]() { RunNetworkThread(strHost.c_str(), strPort.c_str()); });
	return result;
}

void NetworkManagerServer::ReleaseServer()
{
	if (!m_running)
		return;

	m_disconnect = 1;
	SOCKET socket = m_socket;
	m_socket = INVALID_SOCKET;
	::closesocket(socket);
	m_thread->join();

	{
		std::lock_guard<std::recursive_mutex> guard(m_lock);
		for (auto& conn : m_connections)
		{
			conn->ReleaseConnection();
			delete conn;
		}
		m_connections.clear();
	}

	delete m_thread;
	m_thread = nullptr;
}

void NetworkManagerServer::NetworkTickServer()
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);
	for (auto& conn : m_connections)
		if (conn->m_state == CONNECTION_STATE::CONNECTED)
			conn->NetworkTickConnection(this);
	for (auto ite = m_connections.begin(); ite != m_connections.end();)
	{
		if ((*ite)->m_state == CONNECTION_STATE::DISCONNECTED)
			ite = m_connections.erase(ite);
		else
			ite++;
	}
}

int NetworkManagerServer::GetActiveConnections()
{
	int count = 0;

	std::lock_guard<std::recursive_mutex> guard(m_lock);
	for (auto& conn : m_connections)
		if (conn->m_state == CONNECTION_STATE::CONNECTED)
			count++;

	return count;
}

void NetworkManagerServer::SendTo(int client, Message& pkt)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);
	if (client >= 0 && client < m_connections.size())
		m_connections[client]->Send(pkt);
}

void NetworkManagerServer::Broadcast(Message& pkt)
{
	std::lock_guard<std::recursive_mutex> guard(m_lock);
	for (auto& conn : m_connections)
			conn->Send(pkt);
}

void NetworkManagerServer::HandleMessage(ClientConnection* connection, Message& pkt)
{
	for (int i = 0; i < m_connections.size(); i++)
		if (m_connections[i] == connection)
        {
            m_handlers[0](i, pkt);
			return;
        }
    DebuggerPrintf("[NETWORK] Dangling client connection handle: %#016x addr=%s...\n", connection, connection->m_endpointAddr.c_str());
}

void NetworkManagerServer::RegisterHandler(ServerMessageHandler handler)
{
	m_handlers[0] = handler;
}

void NetworkManagerServer::RunNetworkThread(const char* host, const char* port)
{
	m_running = true;

	NetErrResult result;

	DebuggerPrintf("[NETWORK] Starting server on %s...\n", port);

	Bind(host, port);

	if (m_state == CONNECTION_STATE::LISTENING)
		DebuggerPrintf("[NETWORK] Successfully bind port.\n");
	while (m_state == CONNECTION_STATE::LISTENING)
    {
        DebuggerPrintf("[NETWORK] New connection initialized.\n");
		Accept(true);

		if (m_disconnect)
		{
			m_disconnect = 0;
			CloseConnection();
			m_state = CONNECTION_STATE::DISCONNECTED;
			DebuggerPrintf("[NETWORK] Disconnected!\n");
			break;
		}
	}

	DebuggerPrintf("[NETWORK] Server stopped!\n");
	m_running = false;
}

NetErrResult NetworkManagerServer::Bind(const char* host, const char* port)
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::DISCONNECTED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Already listening!";
		return result;
	}

	if (!NET_WINSOCK_INITIALIZED)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Winsock not initialized!";
		return result;
	}

    addrinfo aiHints = {};
    aiHints.ai_family   = AF_INET;
    aiHints.ai_socktype = SOCK_STREAM;
    aiHints.ai_protocol = IPPROTO_TCP;
    aiHints.ai_flags    = AI_PASSIVE;

	// get internal address
    {
        char hostName[256];
        result.m_errCode = ::gethostname(hostName, sizeof(hostName));
        if (result.m_errCode == SOCKET_ERROR) {
            result.m_errCode = ::WSAGetLastError();
            result.m_errMsg = "gethostname failed";
            return result;
        }

        AddrInfoPtr pAIinternal;

        // Resolve the local address and port to be used by the server
        result.m_errCode = ::getaddrinfo(hostName, port, &aiHints, &pAIinternal.pAddrInfo);
        if (result.m_errCode != 0) {
            result.m_errCode = ::WSAGetLastError();
            result.m_errMsg = "getaddrinfo failed";
            return result;
        }

		m_internalAddrs.clear();
        addrinfo* pInfoResult = pAIinternal.pAddrInfo;
        while (pInfoResult)
        {
            sockaddr_in* ipv4 = (sockaddr_in*)pInfoResult->ai_addr;

			m_internalAddrs.push_back(Stringf("%d.%d.%d.%d", (int)ipv4->sin_addr.S_un.S_un_b.s_b1, (int)ipv4->sin_addr.S_un.S_un_b.s_b2, (int)ipv4->sin_addr.S_un.S_un_b.s_b3, (int)ipv4->sin_addr.S_un.S_un_b.s_b4));
            pInfoResult = pInfoResult->ai_next;
        }

		for (auto& addr : m_internalAddrs)
			DebuggerPrintf("Internal address: %s\n", addr.c_str());
	}

    AddrInfoPtr pAIresult;

    // Resolve the local address and port to be used by the server
    result.m_errCode = ::getaddrinfo(host, port, &aiHints, &pAIresult.pAddrInfo);
    if (result.m_errCode != 0) {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = "getaddrinfo failed";
        return result;
    }

	// Create a SOCKET for server

	m_socket = socket(pAIresult.pAddrInfo->ai_family, pAIresult.pAddrInfo->ai_socktype, pAIresult.pAddrInfo->ai_protocol);
	if (m_socket == INVALID_SOCKET) {
		result.m_errCode = ::WSAGetLastError();
		result.m_errMsg = "Failed to create socket";
		return result;
	}

	// Attempt to bind port
	result.m_errCode = ::bind(m_socket, pAIresult.pAddrInfo->ai_addr, (int) pAIresult.pAddrInfo->ai_addrlen);
	if (result.m_errCode == SOCKET_ERROR) {
		result.m_errCode = ::WSAGetLastError();
		result.m_errMsg = "Failed to bind port";
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return result;
	}

	result.m_errCode = ::listen(m_socket, SOMAXCONN);
	if (result.m_errCode == SOCKET_ERROR)
	{
		result.m_errCode = ::WSAGetLastError();
		result.m_errMsg = "Failed to listen on port";
		::closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return result;
	}

	m_state = CONNECTION_STATE::LISTENING;
	return result;
}

NetErrResult NetworkManagerServer::Accept(bool nio)
{
	NetErrResult result;

	if (m_state != CONNECTION_STATE::LISTENING)
	{
		result.m_errCode = -1;
		result.m_errMsg = "Not listening!";
		return result;
	}

	SOCKET clientSocket = INVALID_SOCKET;

	sockaddr_in addrIn;
	int addrLen = (int) sizeof(addrIn);

	clientSocket = ::accept(m_socket, (sockaddr*) &addrIn, &addrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		result.m_errCode = ::WSAGetLastError();
		result.m_errMsg = "Failed to accept new connection";
		CloseConnection();
		return result;
	}

	if (nio)
	{
		u_long mode = 1; // non-blocking
		result.m_errCode = ::ioctlsocket(clientSocket, FIONBIO, &mode);
		if (result.m_errCode != 0)
		{
			result.m_errCode = ::WSAGetLastError();
			result.m_errMsg = "ioctlsocket failed";
			closesocket(clientSocket);
			return result;
		}
	}

	{
		std::lock_guard<std::recursive_mutex> guard(m_lock);

		ClientConnection* connection = new ClientConnection();
		m_connections.push_back(connection);

		connection->m_endpointAddr = Stringf("%d.%d.%d.%d:%d", addrIn.sin_addr.S_un.S_un_b.s_b1, addrIn.sin_addr.S_un.S_un_b.s_b2, addrIn.sin_addr.S_un.S_un_b.s_b3, addrIn.sin_addr.S_un.S_un_b.s_b4, addrIn.sin_port);

        DebuggerPrintf("[NETWORK] New connection accepted: %#016x addr=%s\n", connection, connection->m_endpointAddr.c_str());

		connection->m_socket = clientSocket;
		connection->CreateConnection();
		return result;
	}
}

NetErrResult NetworkManagerServer::CloseConnection()
{
	NetErrResult result;

	if (m_socket == INVALID_SOCKET)
	{
		result.m_errCode = 1;
		result.m_errMsg = "Invalid socket";
		return result;
	}

	result.m_errCode = closesocket(m_socket);
	m_connections.clear();
	m_socket = INVALID_SOCKET;
	m_state = CONNECTION_STATE::DISCONNECTED;
	return result;
}


NetErrResult SessionClient::CreateClient(const char* host, const char* port, int retryTMillis, int maxRetry)
{
    NetErrResult result;
    if (m_running)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Already connected!";
        return result;
    }

    std::string strHost = host;
    std::string strPort = port;

    m_endpointAddr = Stringf("%s:%s", host, port);
    m_thread = new std::thread([this, strHost, strPort, retryTMillis, maxRetry]() { RunNetworkThread(strHost.c_str(), strPort.c_str(), retryTMillis, maxRetry); });
    return result;
}

void SessionClient::ReleaseClient()
{
    if (!m_running)
        return;

    m_disconnect = 1;
    m_thread->join();
    delete m_thread;
    m_thread = nullptr;
}

void SessionClient::NetworkTickClient()
{
    Packet packet;
    while (m_bufferRecv.ReadMessage(packet))
        HandleMessage(packet);

    for (const auto& pkt : m_queue)
        m_bufferSend.WriteMessage(pkt);
    m_queue.clear();
}

void SessionClient::SendToServer(Packet& pkt)
{
    m_queue.push_back(pkt);
}

void SessionClient::HandleMessage(Packet& pkt)
{
    auto ite = m_handlers.find(0);
    if (ite != m_handlers.end())
        (*ite).second(pkt);
}

void SessionClient::RegisterHandler(PacketHandler handler)
{
    m_handlers[0] = handler;
}

void SessionClient::RunNetworkThread(const char* host, const char* port, int retryTimeMillis, int maxRetry)
{
    m_running = true;

    NetErrResult result;

    maxRetry++;
    while (maxRetry > 0 && m_disconnect == 0)
    {
        DebuggerPrintf("[NETWORK] Connecting to %s:%s...\n", host, port);

        {
            const std::lock_guard<std::recursive_mutex> lockSend(m_bufferSend.m_lock);
            const std::lock_guard<std::recursive_mutex> lockRecv(m_bufferRecv.m_lock);
            m_bufferSend.m_data.clear();
            m_bufferRecv.m_data.clear();
        }

        Connect(host, port, true);

        if (m_state == CONNECTION_STATE::ESTABLISHING)
            DebuggerPrintf("[NETWORK] Establishing connection...\n");
        while (m_state == CONNECTION_STATE::ESTABLISHING)
        {
            ValidateConnection();

            if (m_disconnect)
            {
                m_disconnect = 0;
                CloseConnection();
                DebuggerPrintf("[NETWORK] Disconnected!\n");
                goto OUTTER_LOOP_BREAK;
            }
            std::this_thread::yield();
        }

        if (m_state == CONNECTION_STATE::CONNECTED)
            DebuggerPrintf("[NETWORK] Connected!\n");

        while (m_state == CONNECTION_STATE::CONNECTED)
        {
            ClientSendData();
            ClientRecvData();

            if (m_disconnect)
            {
                m_disconnect = 0;
                CloseConnection();
                DebuggerPrintf("[NETWORK] Disconnected!\n");
                goto OUTTER_LOOP_BREAK;
            }
            std::this_thread::yield();
        }


        if (m_disconnect)
        {
            m_disconnect = 0;
            CloseConnection();
            DebuggerPrintf("[NETWORK] Disconnected!\n");
            goto OUTTER_LOOP_BREAK;
        }
        DebuggerPrintf("[NETWORK] Server disconnected!\n");
        std::this_thread::sleep_for(std::chrono::milliseconds(retryTimeMillis));
        maxRetry--;
        DebuggerPrintf("[NETWORK] Retry count left: %d\n", maxRetry);
    }

OUTTER_LOOP_BREAK:
    m_running = false;
}

NetErrResult SessionClient::Connect(const char* host, const char* port, bool nio)
{
    NetErrResult result = {};

    if (m_state != CONNECTION_STATE::DISCONNECTED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Already connected";
        return result;
    }

    if (!NET_WINSOCK_INITIALIZED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Winsock not initialized!";
        return result;
    }

    AddrInfoPtr pAIResult;
    addrinfo  addrinfoHints = {};

    addrinfoHints.ai_family = AF_UNSPEC;
    addrinfoHints.ai_socktype = SOCK_STREAM;
    addrinfoHints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    result.m_errCode = ::getaddrinfo(host, port, &addrinfoHints, &pAIResult.pAddrInfo);
    if (result.m_errCode != 0) {
        result.m_errMsg = "getaddrinfo failed";
        return result;
    }

    // Attempt to connect to the first address returned by the call to getaddrinfo

    // Create a SOCKET for connecting to server
    m_socket = ::socket(pAIResult.pAddrInfo->ai_family, pAIResult.pAddrInfo->ai_socktype, pAIResult.pAddrInfo->ai_protocol);

    if (m_socket == INVALID_SOCKET) {
        result.m_errMsg = Stringf("Error at socket(): %ld", ::WSAGetLastError());
        return result;
    }

    if (nio)
    {
        u_long mode = 1; // non-blocking
        result.m_errCode = ::ioctlsocket(m_socket, FIONBIO, &mode);
        if (result.m_errCode != 0)
        {
            result.m_errMsg = Stringf("ioctlsocket failed: %d", ::WSAGetLastError());
            return result;
        }
    }

    // Connect to server.
    m_state = CONNECTION_STATE::ESTABLISHING;
    result.m_errCode = ::connect(m_socket, pAIResult.pAddrInfo->ai_addr, (int)pAIResult.pAddrInfo->ai_addrlen);
    if (result.m_errCode == SOCKET_ERROR) {
        result.m_errCode = ::WSAGetLastError();
        if (result.m_errCode != WSAEWOULDBLOCK)
        {
            CloseConnection();
            result.m_errMsg = Stringf("Unable to connect to server!");
            return result;
        }

        result.m_errMsg = "establishing!";
        return result;
    }

    sockaddr_in* ipv4 = (sockaddr_in*)pAIResult.pAddrInfo->ai_addr;
    m_endpointAddr = Stringf("%d.%d.%d.%d:%d", ipv4->sin_addr.S_un.S_un_b.s_b1, ipv4->sin_addr.S_un.S_un_b.s_b2, ipv4->sin_addr.S_un.S_un_b.s_b3, ipv4->sin_addr.S_un.S_un_b.s_b4, ipv4->sin_port);
    m_state = CONNECTION_STATE::CONNECTED;
    result.m_errMsg = "success!";
    return result;
}

NetErrResult SessionClient::ClientRecvData()
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::CONNECTED)
    {
        result.m_errCode = 1;
        result.m_errMsg = "Not connected";
        return result;
    }

    bool state = false;
    result = QueryState(m_socket, false, state);
    if (result.m_errCode != 0)
    {
        CloseConnection();
        return result;
    }
    if (!state)
    {
        result.m_errCode = 0;
        result.m_errMsg = "blocking";
        return result;
    }

    char buffer[NET_READ_BUFFER_SIZE] = {};

    int lengthRecv = ::recv(m_socket, buffer, NET_READ_BUFFER_SIZE, 0);
    if (lengthRecv == SOCKET_ERROR)
    {
        result.m_errCode = ::WSAGetLastError();
        if (result.m_errCode != WSAEWOULDBLOCK)
        {
            result.m_errMsg = "Socket error";
            CloseConnection();
            return result;
        }

        // blocking, do nothing
        result.m_errCode = 0;
        return result;
    }
    else if (lengthRecv == 0)
    {
        result.m_errCode = WSAECONNRESET;
        result.m_errMsg = "Connection reset";
        CloseConnection();
        return result;
    }

    DebuggerPrintf("Recv data %d bytes\n", lengthRecv);

    m_bufferRecv.WriteBytes(lengthRecv, buffer);

    return result;
}

NetErrResult SessionClient::ClientSendData()
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::CONNECTED)
    {
        result.m_errCode = 1;
        result.m_errMsg = "Not connected";
        return result;
    }

    bool state = false;
    result = QueryState(m_socket, true, state);
    if (result.m_errCode != 0)
    {
        CloseConnection();
        return result;
    }
    if (!state)
    {
        result.m_errCode = 0;
        result.m_errMsg = "blocking";
        return result;
    }

    char buffer[NET_WRITE_BUFFER_SIZE] = {};
    int length = (int)m_bufferSend.ReadBytes(NET_WRITE_BUFFER_SIZE, buffer);

    if (length == 0)
    {
        result.m_errMsg = "empty";
        return result;
    }

    int lengthSent = ::send(m_socket, buffer, length, 0);
    if (lengthSent == SOCKET_ERROR)
    {
        result.m_errCode = ::WSAGetLastError();
        if (result.m_errCode != WSAEWOULDBLOCK)
        {
            result.m_errMsg = "Socket error";
            CloseConnection();
            return result;
        }

        // blocking, do nothing
        result.m_errCode = 0;
        return result;
    }

    DebuggerPrintf(Stringf("Sent data %d bytes\n", lengthSent).c_str());

    return result;
}

NetErrResult SessionClient::ValidateConnection()
{
    NetErrResult result;

    if (m_socket == INVALID_SOCKET)
    {
        result.m_errCode = 1;
        result.m_errMsg = "Invalid socket";
        return result;
    }

    if (m_state != CONNECTION_STATE::ESTABLISHING)
    {
        result.m_errCode = 1;
        result.m_errMsg = "Invalid state";
        return result;
    }

    bool state = false;
    result = QueryState(m_socket, true, state);
    if (result.m_errCode != 0)
    {
        CloseConnection();
        return result;
    }

    if (state)
        m_state = CONNECTION_STATE::CONNECTED;
    return result;
}

NetErrResult SessionClient::CloseConnection()
{
    NetErrResult result;

    if (m_socket == INVALID_SOCKET)
    {
        result.m_errCode = 1;
        result.m_errMsg = "Invalid socket";
        return result;
    }

    result.m_errCode = ::closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    m_state = CONNECTION_STATE::DISCONNECTED;
    return result;
}

SessionClient::SessionClient()
{
}

SessionClient::~SessionClient()
{
    ReleaseClient();
}

SessionServer::Client::Client()
{
}

SessionServer::Client::~Client()
{
    ReleaseConnection();
}

void SessionServer::Client::CreateConnection()
{
    m_thread = new std::thread([this]() { RunNetworkThread(); });
}

void SessionServer::Client::ReleaseConnection()
{
    if (!m_running)
        return;

    m_disconnect = 1;
    m_thread->join();
    delete m_thread;
    m_thread = nullptr;
}

void SessionServer::Client::NetworkTickConnection(SessionServer* server)
{
    Packet packet;
    while (m_bufferRecv.ReadMessage(packet))
        server->HandleMessage(this, packet);

    for (const auto& pkt : m_queue)
        m_bufferSend.WriteMessage(pkt);
    m_queue.clear();
}

const std::string& SessionServer::Client::GetEndpointAddr() const
{
    return m_endpointAddr;
}

void SessionServer::Client::Disconnect()
{
    ReleaseConnection();
}

void SessionServer::Client::Send(Packet& pkt)
{
    m_queue.push_back(pkt);
}

void SessionServer::Client::RunNetworkThread()
{
    m_running = true;
    m_state = CONNECTION_STATE::CONNECTED;

    while (m_state == CONNECTION_STATE::CONNECTED)
    {
        ClientRecvData();
        ClientSendData();

        if (m_disconnect)
        {
            m_disconnect = false;
            CloseConnection();
            break;
        }
    }

    m_running = false;
}

NetErrResult SessionServer::Client::ClientRecvData()
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::CONNECTED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Not connected!";
        return result;
    }

    bool state = false;
    result = QueryState(m_socket, false, state);
    if (result.m_errCode != 0)
    {
        CloseConnection();
        return result;
    }
    if (!state)
    {
        result.m_errCode = 0;
        result.m_errMsg = "blocking";
        return result;
    }

    char buffer[NET_READ_BUFFER_SIZE] = {};

    int lengthRecv = ::recv(m_socket, buffer, NET_READ_BUFFER_SIZE, 0);
    if (lengthRecv == SOCKET_ERROR)
    {
        result.m_errCode = ::WSAGetLastError();
        if (result.m_errCode != WSAEWOULDBLOCK)
        {
            result.m_errMsg = "Socket error";
            CloseConnection();
            return result;
        }

        // blocking, do nothing
        result.m_errCode = 0;
        return result;
    }
    else if (lengthRecv == 0)
    {
        result.m_errCode = WSAECONNRESET;
        result.m_errMsg = "Connection reset";
        CloseConnection();
        return result;
    }

    m_bufferRecv.WriteBytes(lengthRecv, buffer);
    return result;
}

NetErrResult SessionServer::Client::ClientSendData()
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::CONNECTED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Not connected!";
        return result;
    }

    bool state = false;
    result = QueryState(m_socket, true, state);
    if (result.m_errCode != 0)
    {
        CloseConnection();
        return result;
    }
    if (!state)
    {
        result.m_errCode = 0;
        result.m_errMsg = "blocking";
        return result;
    }

    char buffer[NET_WRITE_BUFFER_SIZE] = {};
    int length = (int)m_bufferSend.ReadBytes(NET_WRITE_BUFFER_SIZE, buffer);

    int lengthSent = ::send(m_socket, buffer, length, 0);
    if (lengthSent == SOCKET_ERROR)
    {
        result.m_errCode = ::WSAGetLastError();
        if (result.m_errCode != WSAEWOULDBLOCK)
        {
            result.m_errMsg = "Socket error";
            CloseConnection();
            return result;
        }

        // blocking, do nothing
        result.m_errCode = 0;
        return result;
    }

    return result;
}

NetErrResult SessionServer::Client::CloseConnection()
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::CONNECTED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Not connected!";
        return result;
    }

    result.m_errCode = ::shutdown(m_socket, SD_SEND);
    if (result.m_errCode == SOCKET_ERROR)
    {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = " Failed to shutdown connection";
        ::closesocket(m_socket);
        m_state = CONNECTION_STATE::DISCONNECTED;
        return result;
    }

    m_state = CONNECTION_STATE::DISCONNECTED;
    return result;
}

SessionServer::SessionServer()
{
}

SessionServer::~SessionServer()
{
    ReleaseServer();
}

NetErrResult SessionServer::CreateServer(const char* host /*= DEFAULT_HOST*/, const char* port /*= DEFAULT_PORT*/)
{
    NetErrResult result;
    if (m_running)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Already running!";
        return result;
    }

    std::string strHost = host;
    std::string strPort = port;

    m_endpointAddr = host;
    m_endpointPort = port;
    m_thread = new std::thread([this, strHost, strPort]() { RunNetworkThread(strHost.c_str(), strPort.c_str()); });
    return result;
}

void SessionServer::ReleaseServer()
{
    if (!m_running)
        return;

    m_disconnect = 1;
    SOCKET socket = m_socket;
    m_socket = INVALID_SOCKET;
    ::closesocket(socket);
    m_thread->join();

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        for (auto& conn : m_connections)
        {
            conn->ReleaseConnection();
            delete conn;
        }
        m_connections.clear();
    }

    delete m_thread;
    m_thread = nullptr;
}

void SessionServer::NetworkTickServer()
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (auto& conn : m_connections)
        if (conn->m_state == CONNECTION_STATE::CONNECTED)
            conn->NetworkTickConnection(this);
    for (auto ite = m_connections.begin(); ite != m_connections.end();)
    {
        if ((*ite)->m_state == CONNECTION_STATE::DISCONNECTED)
            ite = m_connections.erase(ite);
        else
            ite++;
    }
}

int SessionServer::GetActiveConnections()
{
    int count = 0;

    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (auto& conn : m_connections)
        if (conn->m_state == CONNECTION_STATE::CONNECTED)
            count++;

    return count;
}

void SessionServer::SendTo(int client, Packet& pkt)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    if (client >= 0 && client < m_connections.size())
        m_connections[client]->Send(pkt);
}

void SessionServer::Broadcast(Packet& pkt)
{
    std::lock_guard<std::recursive_mutex> guard(m_lock);
    for (auto& conn : m_connections)
        conn->Send(pkt);
}

void SessionServer::HandleMessage(Client* connection, Packet& pkt)
{
    for (int i = 0; i < m_connections.size(); i++)
        if (m_connections[i] == connection)
        {
            m_handlers[0](i, pkt);
            return;
        }
    DebuggerPrintf("[NETWORK] Dangling client connection handle: %#016x addr=%s...\n", connection, connection->m_endpointAddr.c_str());
}

void SessionServer::RegisterHandler(ServerPacketHandler handler)
{
    m_handlers[0] = handler;
}

void SessionServer::RunNetworkThread(const char* host, const char* port)
{
    m_running = true;

    NetErrResult result;

    DebuggerPrintf("[NETWORK] Starting server on %s...\n", port);

    Bind(host, port);

    if (m_state == CONNECTION_STATE::LISTENING)
        DebuggerPrintf("[NETWORK] Successfully bind port.\n");
    while (m_state == CONNECTION_STATE::LISTENING)
    {
        DebuggerPrintf("[NETWORK] New connection initialized.\n");
        Accept(true);

        if (m_disconnect)
        {
            m_disconnect = 0;
            CloseConnection();
            m_state = CONNECTION_STATE::DISCONNECTED;
            DebuggerPrintf("[NETWORK] Disconnected!\n");
            break;
        }
    }

    DebuggerPrintf("[NETWORK] Server stopped!\n");
    m_running = false;
}

NetErrResult SessionServer::Bind(const char* host, const char* port)
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::DISCONNECTED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Already listening!";
        return result;
    }

    if (!NET_WINSOCK_INITIALIZED)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Winsock not initialized!";
        return result;
    }

    addrinfo aiHints = {};
    aiHints.ai_family = AF_INET;
    aiHints.ai_socktype = SOCK_STREAM;
    aiHints.ai_protocol = IPPROTO_TCP;
    aiHints.ai_flags = AI_PASSIVE;

    // get internal address
    {
        char hostName[256];
        result.m_errCode = ::gethostname(hostName, sizeof(hostName));
        if (result.m_errCode == SOCKET_ERROR) {
            result.m_errCode = ::WSAGetLastError();
            result.m_errMsg = "gethostname failed";
            return result;
        }

        AddrInfoPtr pAIinternal;

        // Resolve the local address and port to be used by the server
        result.m_errCode = ::getaddrinfo(hostName, port, &aiHints, &pAIinternal.pAddrInfo);
        if (result.m_errCode != 0) {
            result.m_errCode = ::WSAGetLastError();
            result.m_errMsg = "getaddrinfo failed";
            return result;
        }

        m_internalAddrs.clear();
        addrinfo* pInfoResult = pAIinternal.pAddrInfo;
        while (pInfoResult)
        {
            sockaddr_in* ipv4 = (sockaddr_in*)pInfoResult->ai_addr;

            m_internalAddrs.push_back(Stringf("%d.%d.%d.%d", (int)ipv4->sin_addr.S_un.S_un_b.s_b1, (int)ipv4->sin_addr.S_un.S_un_b.s_b2, (int)ipv4->sin_addr.S_un.S_un_b.s_b3, (int)ipv4->sin_addr.S_un.S_un_b.s_b4));
            pInfoResult = pInfoResult->ai_next;
        }

        for (auto& addr : m_internalAddrs)
            DebuggerPrintf("Internal address: %s\n", addr.c_str());
    }

    AddrInfoPtr pAIresult;

    // Resolve the local address and port to be used by the server
    result.m_errCode = ::getaddrinfo(host, port, &aiHints, &pAIresult.pAddrInfo);
    if (result.m_errCode != 0) {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = "getaddrinfo failed";
        return result;
    }

    // Create a SOCKET for server

    m_socket = socket(pAIresult.pAddrInfo->ai_family, pAIresult.pAddrInfo->ai_socktype, pAIresult.pAddrInfo->ai_protocol);
    if (m_socket == INVALID_SOCKET) {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = "Failed to create socket";
        return result;
    }

    // Attempt to bind port
    result.m_errCode = ::bind(m_socket, pAIresult.pAddrInfo->ai_addr, (int)pAIresult.pAddrInfo->ai_addrlen);
    if (result.m_errCode == SOCKET_ERROR) {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = "Failed to bind port";
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return result;
    }

    result.m_errCode = ::listen(m_socket, SOMAXCONN);
    if (result.m_errCode == SOCKET_ERROR)
    {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = "Failed to listen on port";
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return result;
    }

    m_state = CONNECTION_STATE::LISTENING;
    return result;
}

NetErrResult SessionServer::Accept(bool nio)
{
    NetErrResult result;

    if (m_state != CONNECTION_STATE::LISTENING)
    {
        result.m_errCode = -1;
        result.m_errMsg = "Not listening!";
        return result;
    }

    SOCKET clientSocket = INVALID_SOCKET;

    sockaddr_in addrIn;
    int addrLen = (int)sizeof(addrIn);

    clientSocket = ::accept(m_socket, (sockaddr*)&addrIn, &addrLen);
    if (clientSocket == INVALID_SOCKET)
    {
        result.m_errCode = ::WSAGetLastError();
        result.m_errMsg = "Failed to accept new connection";
        CloseConnection();
        return result;
    }

    if (nio)
    {
        u_long mode = 1; // non-blocking
        result.m_errCode = ::ioctlsocket(clientSocket, FIONBIO, &mode);
        if (result.m_errCode != 0)
        {
            result.m_errCode = ::WSAGetLastError();
            result.m_errMsg = "ioctlsocket failed";
            closesocket(clientSocket);
            return result;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);

        Client* connection = new Client();
        m_connections.push_back(connection);

        connection->m_endpointAddr = Stringf("%d.%d.%d.%d:%d", addrIn.sin_addr.S_un.S_un_b.s_b1, addrIn.sin_addr.S_un.S_un_b.s_b2, addrIn.sin_addr.S_un.S_un_b.s_b3, addrIn.sin_addr.S_un.S_un_b.s_b4, addrIn.sin_port);

        DebuggerPrintf("[NETWORK] New connection accepted: %#016x addr=%s\n", connection, connection->m_endpointAddr.c_str());

        connection->m_socket = clientSocket;
        connection->CreateConnection();
        return result;
    }
}

NetErrResult SessionServer::CloseConnection()
{
    NetErrResult result;

    if (m_socket == INVALID_SOCKET)
    {
        result.m_errCode = 1;
        result.m_errMsg = "Invalid socket";
        return result;
    }

    result.m_errCode = closesocket(m_socket);
    m_connections.clear();
    m_socket = INVALID_SOCKET;
    m_state = CONNECTION_STATE::DISCONNECTED;
    return result;
}

