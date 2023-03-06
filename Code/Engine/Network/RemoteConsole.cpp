#include "Engine/Network/RemoteConsole.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Network/NetworkSystem.hpp"

#include "Engine/Renderer/DebugRender.hpp"


bool Command_RCHost(EventArgs& args)
{
    auto port = args.GetValue("port", "3121");

    g_remoteConsole->Host(port);
    return true;
}

bool Command_RCKick(EventArgs& args)
{
    auto index = args.GetValue("index", 0);

    g_remoteConsole->Kick(index);
    return true;
}

bool Command_RCJoin(EventArgs& args)
{
    auto host = args.GetValue("host", "127.0.0.1");
    auto port = args.GetValue("port", "3121");

    g_remoteConsole->Join(host, port);
    return true;
}

bool Command_RCSend(EventArgs& args)
{
    auto cmd = args.GetValue("cmd", "");

    g_remoteConsole->Send(cmd);
    return true;
}

bool Command_RCLeave(EventArgs& args)
{
    UNUSED(args);
    g_remoteConsole->Leave();
    return true;
}

bool Command_RCEcho(EventArgs& args)
{
    auto index = args.GetValue("index", 0);
    auto msg = args.GetValue("msg", "(empty)");
    auto all = args.GetValue("broadcast", false);

    g_remoteConsole->Echo(all ? -1 : index, msg);
    return true;
}

bool Command_RCDebug(EventArgs& args)
{
    UNUSED(args);
    g_remoteConsole->ToggleDebug();
    return true;
}

void RCHandleMessage(Message& pkt)
{
    if (g_remoteConsole)
        g_remoteConsole->HandleMessage(pkt);
}

void RCHandleClientMessage(int index, Message& pkt)
{
    if (g_remoteConsole)
        g_remoteConsole->HandleClientMessage(index, pkt);
}

void RCMessageSink(const Rgba8& color, const std::string& message)
{
    UNUSED(color);
    if (g_remoteConsole)
        g_remoteConsole->ConsoleMessageSink(message);
}


const char* GetName(RemoteConsole::State enumRCState)
{
    static const char* names[(int)RemoteConsole::State::SIZE] = { 
        "DISCONNECTED",
        "ESTABLISHING_CLIENT",
        "ESTABLISHING_SERVER",
        "CONNECTED",
        "HOSTING",
        "IDLE"
    };
    return names[(int)enumRCState];
}

RemoteConsole* g_remoteConsole = nullptr;

std::string RC_HEADER = "[Remote Console] ";

RemoteConsole::RemoteConsole(const RemoteConsoleConfig& config)
    : m_config(config)
    , m_console(config.m_console)
    , m_client(new NetworkManagerClient())
    , m_server(new NetworkManagerServer())
{
    m_client->RegisterHandler(RCHandleMessage);
    m_server->RegisterHandler(RCHandleClientMessage);
}

RemoteConsole::~RemoteConsole()
{
    delete m_client;
    delete m_server;
}

void RemoteConsole::Startup()
{
    g_theEventSystem->SubscribeEventCallbackFunction("RCHost",  Command_RCHost);
    g_theEventSystem->SubscribeEventCallbackFunction("RCKick",  Command_RCKick);
    g_theEventSystem->SubscribeEventCallbackFunction("RCJoin",  Command_RCJoin);
    g_theEventSystem->SubscribeEventCallbackFunction("RCSend",  Command_RCSend);
    g_theEventSystem->SubscribeEventCallbackFunction("RCLeave", Command_RCLeave);
    g_theEventSystem->SubscribeEventCallbackFunction("RCEcho",  Command_RCEcho);
    g_theEventSystem->SubscribeEventCallbackFunction("RCDebug", Command_RCDebug);

    m_console->SetNetMessageSink(RCMessageSink);
    m_console->SetBannedCmds(PERMISSION_REMOTE, {
        "RCHost",
        "RCKick",
        "RCJoin",
        "RCSend",
        "RCLeave",
        "RCEcho",
        "RCDebug"
        });
}

void RemoteConsole::BeginFrame()
{
    DebugAddMessage(RC_HEADER + GetStateName(), 0);

    if (m_state == State::DISCONNECTED)
    {
        m_client->CreateClient(m_config.m_addr.c_str(), m_config.m_port.c_str());
        m_state = State::ESTABLISHING_CLIENT;
        m_idleWatch.Start(3.0f);
        m_local = true;
    }
    else if (m_state == State::ESTABLISHING_CLIENT)
    {
        if (m_client->m_state == CONNECTION_STATE::CONNECTED)
        {
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Connected to %s", m_client->m_endpointAddr.c_str()));
            m_state = State::CONNECTED;
            return;
        }

        if (!m_idleWatch.HasDurationElapsed())
            return;
        m_idleWatch.Stop();

        if (m_client->m_state == CONNECTION_STATE::DISCONNECTED || m_client->m_state == CONNECTION_STATE::ESTABLISHING)
        {
            m_client->ReleaseClient();

            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Connection failed to %s", m_client->m_endpointAddr.c_str()));
            m_state = State::IDLE;

            if (m_local)
            {
                m_local = false;
                m_server->CreateServer("0.0.0.0", m_config.m_port.c_str());
                m_state = State::ESTABLISHING_SERVER;
                m_idleWatch.Start(1.0f);
            }
        }
    }
    else if (m_state == State::ESTABLISHING_SERVER)
    {
        if (m_server->m_state == CONNECTION_STATE::LISTENING)
        {
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Hosting on %s:%s", m_server->m_endpointAddr.c_str(), m_server->m_endpointPort.c_str()));
            m_state = State::HOSTING;
            return;
        }

        if (!m_idleWatch.HasDurationElapsed())
            return;
        m_idleWatch.Stop();

        if (m_server->m_state == CONNECTION_STATE::DISCONNECTED || m_server->m_state == CONNECTION_STATE::ESTABLISHING)
        {
            m_server->ReleaseServer();

            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Hosting failed on %s", m_server->m_internalAddrs[0].c_str()));
            m_state = State::IDLE;
        }
    }
    else if (m_state == State::CONNECTED)
    {
        if (m_client->m_state == CONNECTION_STATE::DISCONNECTED)
        {
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Disconnected from %s", m_client->m_endpointAddr.c_str()));
            m_state = State::IDLE;

            if (m_local)
            {
                m_local = false;
                m_server->CreateServer("0.0.0.0", m_config.m_port.c_str());
                m_state = State::ESTABLISHING_SERVER;
                m_idleWatch.Start(1.0);
            }
        }
        else
        {
            UpdateNetClient();
        }
    }
    else if (m_state == State::HOSTING)
    {
        if (m_server->m_state == CONNECTION_STATE::DISCONNECTED)
        {
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Stopped hosting on %s", m_server->m_internalAddrs[0].c_str()));
            m_state = State::IDLE;
        }
        else
        {
            UpdateNetServer();
        }
    }
}

void RemoteConsole::EndFrame()
{

}

void RemoteConsole::Shutdown()
{
    m_client->ReleaseClient();
    m_server->ReleaseServer();
    m_state = State::DISCONNECTED;

    g_theEventSystem->UnsubscribeEventCallbackFunction("RCHost",  Command_RCHost);
    g_theEventSystem->UnsubscribeEventCallbackFunction("RCKick",  Command_RCKick);
    g_theEventSystem->UnsubscribeEventCallbackFunction("RCJoin",  Command_RCJoin);
    g_theEventSystem->UnsubscribeEventCallbackFunction("RCSend",  Command_RCSend);
    g_theEventSystem->UnsubscribeEventCallbackFunction("RCLeave", Command_RCLeave);
    g_theEventSystem->UnsubscribeEventCallbackFunction("RCEcho",  Command_RCEcho);
    g_theEventSystem->UnsubscribeEventCallbackFunction("RCDebug", Command_RCDebug);

    m_console->SetNetMessageSink(nullptr);
    m_console->SetBannedCmds(PERMISSION_REMOTE, {});
}

void RemoteConsole::UpdateNetClient()
{
    DebugAddMessage(RC_HEADER + Stringf("Server: %s", m_client->m_endpointAddr.c_str()), 0);

    m_client->NetworkTickClient();
}

void RemoteConsole::UpdateNetServer()
{
    for (auto& addr : m_server->m_internalAddrs)
        DebugAddMessage(RC_HEADER + Stringf("Address: %s", addr.c_str()), 0);
    int index = 0;
    for (auto* conn : m_server->m_connections)
        DebugAddMessage(RC_HEADER + Stringf("[#%d]: %s", index++, conn->GetEndpointAddr().c_str()), 0);

    m_server->NetworkTickServer();
}

void RemoteConsole::HandleMessage(Message& pkt)
{
    if (pkt.m_isEcho)
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("[ECHO] %s", pkt.m_msg.c_str()));
    }
    else
    {
        m_console->AddLine(DevConsole::LOG_WARN, Stringf("Server request command: %s", pkt.m_msg.c_str()));
    }
}

void RemoteConsole::HandleClientMessage(int index, Message& pkt)
{
    auto connection = m_server->m_connections[index];

    if (m_debug)
    {
        m_conversation = connection;
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("[DEBUG] Received: #%d %s %s", index, pkt.m_isEcho ? "(ECHO)" : "(CMD)", pkt.m_msg.c_str()));
        m_conversation = nullptr;
        return;
    }

    if (pkt.m_isEcho)
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("[ECHO] (#%d) %s", index, pkt.m_msg.c_str()));
    }
    else
    {
        m_conversation = connection;
        m_console->Execute(pkt.m_msg, PermissionLevel::PERMISSION_REMOTE);
        m_conversation = nullptr;
    }
}

void RemoteConsole::Host(const std::string& port)
{
    if (m_state == State::DISCONNECTED || m_state == State::IDLE)
    {
        m_state = State::ESTABLISHING_SERVER;
        m_idleWatch.Start(1.0);
        m_server->CreateServer("0.0.0.0", port.c_str());
        m_console->AddLine(DevConsole::LOG_INFO, "Establishing connection...");
    }
    else
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal state (%s)", GetStateName()));
    }
}

void RemoteConsole::Kick(int index)
{
    if (m_state == State::HOSTING)
    {
        if (index < 0)
        {
            int idx = 0;
            for (auto conn : m_server->m_connections)
            {
                conn->Disconnect();
                m_console->AddLine(DevConsole::LOG_INFO, Stringf("Disconnected #%d", idx++));
            }
        }
        else if (index >= 0 && index < m_server->m_connections.size())
        {
            m_server->m_connections[index]->Disconnect();
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Disconnected #%d", index));
        }
        else
        {
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal index (%d)", index));
        }
    }
    else
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal state (%s)", GetStateName()));
    }
}

void RemoteConsole::Join(const std::string& addr, const std::string& port)
{
    if (m_state == State::DISCONNECTED || m_state == State::IDLE)
    {
        m_state = State::ESTABLISHING_CLIENT;
        m_idleWatch.Start(3.0);
        m_client->CreateClient(addr.c_str(), port.c_str());
        m_console->AddLine(DevConsole::LOG_INFO, "Establishing connection...");
    }
    else
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal state (%s)", GetStateName()));
    }
}

void RemoteConsole::Send(const std::string& message)
{
    if (m_state == State::CONNECTED)
    {
        Message pkt;
        pkt.m_isEcho = false;
        pkt.m_msg = message;
        m_client->SendToServer(pkt);
        m_console->AddLine(DevConsole::LOG_INFO, "Command has been sent");
    }
    else
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal state (%s)", GetStateName()));
    }
}

void RemoteConsole::Leave()
{
    if (m_state == State::IDLE || m_state == State::DISCONNECTED)
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal state (%s)", GetStateName()));
    }
    else
    {
        m_local = false;
        if (m_client->m_state != CONNECTION_STATE::DISCONNECTED)
        {
            m_client->ReleaseClient();
            m_console->AddLine(DevConsole::LOG_INFO, "Disconnecting client...");
        }
        if (m_server->m_state != CONNECTION_STATE::DISCONNECTED)
        {
            m_server->ReleaseServer();
            m_console->AddLine(DevConsole::LOG_INFO, "Disconnecting server...");
        }
    }
}

void RemoteConsole::Echo(int index, const std::string& msg)
{
    Message pkt;
    pkt.m_isEcho = true;
    pkt.m_msg = msg;

    if (m_state == State::CONNECTED)
    {
        m_client->SendToServer(pkt);
        m_console->AddLine(DevConsole::LOG_INFO, "Message has been sent");
    }
    else if (m_state == State::HOSTING)
    {
        if (index < 0)
        {
            for (auto conn : m_server->m_connections)
            {
                conn->Send(pkt);
            }
            m_console->AddLine(DevConsole::LOG_INFO, "Broadcast message has been sent");
        }
        else if (index >= 0 && index < m_server->m_connections.size())
        {
            m_server->m_connections[index]->Send(pkt);
            m_console->AddLine(DevConsole::LOG_INFO, "Message has been sent");
        }
        else
        {
            m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal index (%d)", index));
        }
    }
    else
    {
        m_console->AddLine(DevConsole::LOG_INFO, Stringf("Failed: illegal state (%s)", GetStateName()));
    }
}

void RemoteConsole::ToggleDebug()
{
    m_debug = !m_debug;
    m_console->AddLine(DevConsole::LOG_INFO, Stringf("Debug toggled %s", m_debug ? "ON" : "OFF"));
}

RemoteConsole::State RemoteConsole::GetState() const
{
    return m_state;
}

const char* RemoteConsole::GetStateName() const
{
    return GetName(m_state);
}

void RemoteConsole::ConsoleMessageSink(const std::string& msg)
{
    if (m_conversation)
    {
        Message pkt;
        pkt.m_isEcho = true;
        pkt.m_msg = msg;
        m_conversation->Send(pkt);
    }
}
