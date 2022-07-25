#pragma once

#include "Engine/Core/NamedStrings.hpp"
#include <map>
#include <mutex>

// code review

typedef NamedStrings EventArgs;

typedef bool (*EventCallbackFunction)(EventArgs& args);

struct EventSubscription
{
public:
    EventSubscription(const EventCallbackFunction& functionPtr);
    ~EventSubscription();

public:
    EventCallbackFunction m_functionPtr;
};

typedef std::vector<EventSubscription> SubscriptionList;

typedef std::map<std::string, SubscriptionList> EventSubscriptionMap;

struct EventSystemConfig
{
};

class EventSystem
{
public:
    EventSystem(const EventSystemConfig& theConfig);
    ~EventSystem();

    // lifecycle
    void Startup();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    // functions
    void SubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFunction functionPtr);
    void UnsubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFunction functionPtr);
    bool FireEvent(const std::string& eventName, EventArgs& args) const;
    bool FireEvent(const std::string& eventName) const;
    void GetRegisteredEventNames(std::vector<std::string>& outNames) const;

private:
    const EventSystemConfig    m_theConfig;
    EventSubscriptionMap       m_subscriptionListMap;
    mutable std::mutex         m_subscriptionMutex;
};

