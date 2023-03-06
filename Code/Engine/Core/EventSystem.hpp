#pragma once

#include "Engine/Core/NamedStrings.hpp"
#include <map>
#include <mutex>

#include <functional>

#include "NamedProperties.hpp"

using EventArgs = NamedProperties;

using EventCallbackFP = bool (*)(EventArgs& args);

using EventCallbackFunction = EventCallbackFP; // for compatibility

using EventCallback = std::function<bool(EventArgs&)>;

using EventHandler = uint32_t;

using Subscriber = const void*;

constexpr EventHandler INVALID_HANDLER = 0;

class EventSystem;
class SubscriptionList;

class EventSubscription
{
    friend class EventSystem;
    friend class SubscriptionList;

public:
    EventSubscription(EventHandler handler, Subscriber subscriber, EventCallbackFP functionPtr);
    EventSubscription(EventHandler handler, Subscriber subscriber, const EventCallback& callback);

private:
    EventHandler    m_handler;
    Subscriber      m_subscriber;
    EventCallbackFP m_functionPtr;
    EventCallback   m_callback;
};

class SubscriptionList
{
	friend class EventSystem;

public:
    EventHandler Subscribe(EventCallbackFP callback, Subscriber subscriber = nullptr);
    EventHandler Subscribe(const EventCallback& callback, Subscriber subscriber = nullptr);
    void Unsubscribe(EventCallbackFP fp);
    void Unsubscribe(EventHandler handle);
    void Unsubscribe(Subscriber subscriber);
    bool Dispatch(EventArgs& args) const;

private:
    std::vector<EventSubscription> m_list;
    EventHandler m_nextHandler = 1;
};

typedef std::map<std::string, SubscriptionList> EventSubscriptionMap;

struct EventSystemConfig
{
};

class EventSystem
{
public:
    EventSystem(const EventSystemConfig& theConfig);

    // lifecycle
    void Startup();
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    // functions
    EventHandler SubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFP functionPtr, Subscriber subscriber = nullptr);
    void UnsubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFP functionPtr);
	EventHandler Subscribe(const std::string& eventName, const EventCallback& callback, Subscriber subscriber = nullptr);
	void Unsubscribe(const std::string& eventName, EventHandler handle);
	void Unsubscribe(const std::string& eventName, Subscriber subscriber);
	void Unsubscribe(Subscriber subscriber);
    bool FireEvent(const std::string& eventName, EventArgs& args) const;
    bool FireEvent(const std::string& eventName) const;
    void GetRegisteredEventNames(std::vector<std::string>& outNames) const;

private:
    const EventSystemConfig    m_theConfig;
    EventSubscriptionMap       m_subscriptionListMap;
    mutable std::mutex         m_subscriptionMutex;
};

class EventRecipient
{
protected:
    EventRecipient(EventSystem& eventSystem);

public:
    virtual ~EventRecipient();

    EventRecipient(const EventRecipient&) = delete;
    EventRecipient(EventRecipient&&) = delete;

    void operator=(const EventRecipient&) = delete;
    void operator=(EventRecipient&&) = delete;

private:
    EventSystem& m_eventSystem;
};
