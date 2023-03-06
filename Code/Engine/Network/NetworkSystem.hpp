#pragma once

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Network/Message.hpp"
#include "Engine/Network/Packet.hpp"

#include <vector>
#include <mutex>
#include <thread>
#include <string>
#include <map>
#include <atomic>
#include <functional>

constexpr const char* DEFAULT_SERVER = "127.0.0.1"; // First in local
constexpr const char* DEFAULT_HOST = "0.0.0.0"; // First in local
constexpr const char* DEFAULT_PORT = "25564"; // I'm not minecraft
constexpr const int   DEFAULT_RETRY_TIME = 1500;
constexpr const int   DEFAULT_RETRY_COUNT = 3;

extern unsigned short (*HTONS)(unsigned short);
extern unsigned short (*NTOHS)(unsigned short);
extern unsigned long (*HTONL)(unsigned long);
extern unsigned long (*NTOHL)(unsigned long);


struct NetErrResult
{
	int m_errCode = 0;
	std::string m_errMsg = "success";
};


NetErrResult StartupNetworking();
NetErrResult StopNetworking();


using UINT_PTR = unsigned long long;
using SOCKET = UINT_PTR;
#define INVALID_SOCKET  (SOCKET)(~0)


enum class CONNECTION_STATE
{
	DISCONNECTED,
	ESTABLISHING,
	CONNECTED,
	LISTENING,
};

const char* GetNameFromType(CONNECTION_STATE type);
CONNECTION_STATE GetTypeByName(const char* name, CONNECTION_STATE defaultType);


using MessageHandler = std::function<void(Message& packet)>;
using ServerMessageHandler = std::function<void(int client, Message& packet)>;


class NetworkManagerClient
{
public:
	std::thread* m_thread = nullptr;
	std::atomic_int m_disconnect = 0;
	std::atomic_int m_running = 0;
	std::atomic<CONNECTION_STATE> m_state = CONNECTION_STATE::DISCONNECTED;
	Stopwatch m_timeoutWatch;
	bool m_canRead = false;
	bool m_canWrite = false;

	std::string m_endpointAddr;
	SOCKET m_socket = INVALID_SOCKET;

	MessageBuffer m_bufferSend;
	MessageBuffer m_bufferRecv;

	std::vector<Message> m_queue;
	std::map<int, MessageHandler> m_handlers;
	
	NetworkManagerClient();
	NetworkManagerClient(const NetworkManagerClient& copyFrom) = delete;
	~NetworkManagerClient();

	NetErrResult CreateClient(const char* host = DEFAULT_SERVER, const char* port = DEFAULT_PORT, int retryTMillis = DEFAULT_RETRY_TIME, int maxRetry = DEFAULT_RETRY_COUNT);
	void ReleaseClient();
	void NetworkTickClient();

	void SendToServer(Message& pkt);
	void HandleMessage(Message& pkt);
	void RegisterHandler(MessageHandler handler);

private:
	void RunNetworkThread(const char* host, const char* port, int retryTMillis, int maxRetry);
	NetErrResult Connect(const char* host, const char* port, bool nio);
	NetErrResult ClientRecvData();
	NetErrResult ClientSendData();
	NetErrResult ValidateConnection();
	NetErrResult CloseConnection();
};


class NetworkManagerServer
{
public:
	struct ClientConnection
	{
		friend class NetworkManagerServer;

	private:
		std::thread* m_thread = nullptr;
		std::atomic_int m_disconnect = 0;
		std::atomic_int m_running = 0;
		std::atomic<CONNECTION_STATE> m_state = CONNECTION_STATE::DISCONNECTED;

		std::string m_endpointAddr;
		SOCKET m_socket = INVALID_SOCKET;

		MessageBuffer m_bufferSend;
		MessageBuffer m_bufferRecv;

		std::vector<Message> m_queue;

	private:
		ClientConnection();
		ClientConnection(const ClientConnection& copyFrom) = delete;
		~ClientConnection();

		void CreateConnection();
        void ReleaseConnection();
        void NetworkTickConnection(NetworkManagerServer* server);

	public:
		const std::string& GetEndpointAddr() const;
		void Disconnect();
		void Send(Message& pkt);

	private:
		void RunNetworkThread();
		NetErrResult ClientRecvData();
		NetErrResult ClientSendData();
		NetErrResult CloseConnection();
	};

public:
	std::recursive_mutex m_lock;
    std::thread* m_thread = nullptr;
    std::atomic_int m_disconnect = 0;
    std::atomic_int m_running = 0;
	std::atomic<CONNECTION_STATE> m_state = CONNECTION_STATE::DISCONNECTED;

	std::string m_endpointAddr;
	std::string m_endpointPort;
	std::vector<std::string> m_internalAddrs;
	std::atomic<SOCKET> m_socket = INVALID_SOCKET;

	std::vector<ClientConnection*> m_connections;

	std::map<int, ServerMessageHandler> m_handlers;

	NetworkManagerServer();
	~NetworkManagerServer();

	NetErrResult CreateServer(const char* host = DEFAULT_HOST, const char* port = DEFAULT_PORT);

	void ReleaseServer();
	void NetworkTickServer();

	int  GetActiveConnections();
	void SendTo(int client, Message& pkt);
	void Broadcast(Message& pkt);
	void HandleMessage(ClientConnection* connection, Message& pkt);
	void RegisterHandler(ServerMessageHandler handler);

private:
	void RunNetworkThread(const char* host, const char* port);
	NetErrResult Bind(const char* host, const char* port);
	NetErrResult Accept(bool nio);
	NetErrResult CloseConnection();
};


class Packet;

using PacketHandler = std::function<void(Packet& packet)>;
using ServerPacketHandler = std::function<void(int client, Packet& packet)>;


class SessionClient
{
public:
    std::thread* m_thread = nullptr;
    std::atomic_int m_disconnect = 0;
    std::atomic_int m_running = 0;
    std::atomic<CONNECTION_STATE> m_state = CONNECTION_STATE::DISCONNECTED;
    Stopwatch m_timeoutWatch;
    bool m_canRead = false;
    bool m_canWrite = false;

    std::string m_endpointAddr;
    SOCKET m_socket = INVALID_SOCKET;

    PacketBuffer m_bufferSend;
    PacketBuffer m_bufferRecv;

    std::vector<Packet> m_queue;
    std::map<int, PacketHandler> m_handlers;

    SessionClient();
    SessionClient(const SessionClient& copyFrom) = delete;
    SessionClient(const SessionClient&& moveFrom) = delete;
    ~SessionClient();

    void operator=(const SessionClient& copyFrom) = delete;
    void operator=(const SessionClient&& moveFrom) = delete;

    NetErrResult CreateClient(const char* host = DEFAULT_SERVER, const char* port = DEFAULT_PORT, int retryTMillis = DEFAULT_RETRY_TIME, int maxRetry = DEFAULT_RETRY_COUNT);
    void ReleaseClient();
    void NetworkTickClient();

    void SendToServer(Packet& pkt);
    void HandleMessage(Packet& pkt);
    void RegisterHandler(PacketHandler handler);

private:
    void RunNetworkThread(const char* host, const char* port, int retryTMillis, int maxRetry);
    NetErrResult Connect(const char* host, const char* port, bool nio);
    NetErrResult ClientRecvData();
    NetErrResult ClientSendData();
    NetErrResult ValidateConnection();
    NetErrResult CloseConnection();
};


class SessionServer
{
public:
    struct Client
    {
        friend class SessionServer;

    private:
        std::thread* m_thread = nullptr;
        std::atomic_int m_disconnect = 0;
        std::atomic_int m_running = 0;
        std::atomic<CONNECTION_STATE> m_state = CONNECTION_STATE::DISCONNECTED;

        std::string m_endpointAddr;
        SOCKET m_socket = INVALID_SOCKET;

        PacketBuffer m_bufferSend;
        PacketBuffer m_bufferRecv;

        std::vector<Packet> m_queue;

    private:
        Client();
        Client(const Client& copyFrom) = delete;
        Client(const Client&& moveFrom) = delete;
        ~Client();

        void operator=(const Client& copyFrom) = delete;
        void operator=(const Client&& moveFrom) = delete;

        void CreateConnection();
        void ReleaseConnection();
        void NetworkTickConnection(SessionServer* server);

    public:
        const std::string& GetEndpointAddr() const;
        void Disconnect();
        void Send(Packet& pkt);

    private:
        void RunNetworkThread();
        NetErrResult ClientRecvData();
        NetErrResult ClientSendData();
        NetErrResult CloseConnection();
    };

public:
    std::recursive_mutex m_lock;
    std::thread* m_thread = nullptr;
    std::atomic_int m_disconnect = 0;
    std::atomic_int m_running = 0;
    std::atomic<CONNECTION_STATE> m_state = CONNECTION_STATE::DISCONNECTED;

    std::string m_endpointAddr;
    std::string m_endpointPort;
    std::vector<std::string> m_internalAddrs;
    std::atomic<SOCKET> m_socket = INVALID_SOCKET;

    std::vector<Client*> m_connections;

    std::map<int, ServerPacketHandler> m_handlers;

    SessionServer();
    SessionServer(const SessionServer& copyFrom) = delete;
    SessionServer(const SessionServer&& moveFrom) = delete;
    ~SessionServer();

    void operator=(const SessionServer& copyFrom) = delete;
    void operator=(const SessionServer&& moveFrom) = delete;

    NetErrResult CreateServer(const char* host = DEFAULT_HOST, const char* port = DEFAULT_PORT);

    void ReleaseServer();
    void NetworkTickServer();

    int  GetActiveConnections();
    void SendTo(int client, Packet& pkt);
    void Broadcast(Packet& pkt);
    void HandleMessage(Client* connection, Packet& pkt);
    void RegisterHandler(ServerPacketHandler handler);

private:
    void RunNetworkThread(const char* host, const char* port);
    NetErrResult Bind(const char* host, const char* port);
    NetErrResult Accept(bool nio);
    NetErrResult CloseConnection();
};
