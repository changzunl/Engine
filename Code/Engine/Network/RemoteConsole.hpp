#pragma once

#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/Stopwatch.hpp"

#include <string>

class DevConsole;
class EventSystem;

struct RemoteConsoleConfig
{
    DevConsole* m_console = nullptr;
    EventSystem* m_eventbus;
    std::string m_addr = "127.0.0.1";
    std::string m_port = "3121";
};

class Message;

class RemoteConsole
{
public:
    enum class State
    {
        DISCONNECTED,
        ESTABLISHING_CLIENT,
        ESTABLISHING_SERVER,
        CONNECTED,
        HOSTING,
        IDLE,
        SIZE,
    };

    RemoteConsole(const RemoteConsoleConfig& config);
    ~RemoteConsole();

    // lifecycle
    void Startup();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    void UpdateNetClient();
    void UpdateNetServer();

    void HandleMessage(Message& pkt);
    void HandleClientMessage(int index, Message& pkt);

    void Host(const std::string& port);
    void Kick(int index);
    void Join(const std::string& addr, const std::string& port);
    void Send(const std::string& message);
    void Leave();
    void Echo(int index, const std::string& msg);
    void ToggleDebug();

    State GetState() const;
    const char* GetStateName() const;

    void ConsoleMessageSink(const std::string& msg);

private:
    RemoteConsoleConfig m_config;
    DevConsole* m_console;
    NetworkManagerClient *const m_client;
    NetworkManagerServer *const m_server;
    State m_state = State::DISCONNECTED;
    bool m_local = false;
    bool m_debug = false;

    Stopwatch m_idleWatch;
    NetworkManagerServer::ClientConnection* m_conversation = nullptr;
};


const char* GetName(RemoteConsole::State enumRCState);

extern RemoteConsole* g_remoteConsole;

