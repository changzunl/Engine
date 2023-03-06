#include "EventSystem.hpp"

EventSystem* g_theEventSystem = nullptr;


EventSubscription::EventSubscription(EventHandler handler, Subscriber subscriber, EventCallbackFP functionPtr)
    : m_handler(handler)
    , m_subscriber(subscriber)
    , m_functionPtr(functionPtr)
    , m_callback()
{

}

EventSubscription::EventSubscription(EventHandler handler, Subscriber subscriber, const EventCallback& callback)
	: m_handler(handler)
	, m_subscriber(subscriber)
	, m_functionPtr(nullptr)
	, m_callback(callback)
{

}

EventSystem::EventSystem(const EventSystemConfig& theConfig)
    : m_theConfig(theConfig)
{
}

void EventSystem::Startup()
{

}

void EventSystem::BeginFrame()
{

}

void EventSystem::EndFrame()
{

}

void EventSystem::Shutdown()
{

}

EventHandler EventSystem::SubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFP functionPtr, Subscriber subscriber)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

    return m_subscriptionListMap[tolower(eventName)].Subscribe(functionPtr, subscriber);
}

void EventSystem::UnsubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFP functionPtr)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

	auto ite = m_subscriptionListMap.find(tolower(eventName));
	if (ite != m_subscriptionListMap.end())
		ite->second.Unsubscribe(functionPtr);
}

EventHandler EventSystem::Subscribe(const std::string& eventName, const EventCallback& callback, Subscriber subscriber)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

	return m_subscriptionListMap[tolower(eventName)].Subscribe(callback, subscriber);
}

void EventSystem::Unsubscribe(const std::string& eventName, EventHandler handle)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

	auto ite = m_subscriptionListMap.find(tolower(eventName));
	if (ite != m_subscriptionListMap.end())
		ite->second.Unsubscribe(handle);
}

void EventSystem::Unsubscribe(const std::string& eventName, Subscriber subscriber)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

	auto ite = m_subscriptionListMap.find(tolower(eventName));
	if (ite != m_subscriptionListMap.end())
		ite->second.Unsubscribe(subscriber);
}

void EventSystem::Unsubscribe(Subscriber subscriber)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

	for (auto& pair : m_subscriptionListMap)
	{
		pair.second.Unsubscribe(subscriber);
	}
}

bool EventSystem::FireEvent(const std::string& eventName, EventArgs& args) const
{
	std::vector<EventSubscription> list;

	{
		std::lock_guard<std::mutex> guard(m_subscriptionMutex);

		auto ite = m_subscriptionListMap.find(tolower(eventName));
		if (ite != m_subscriptionListMap.end())
		{
			list = ite->second.m_list;
		}
	}

	for (const auto& subscription : list)
	{
		bool consumed;
		if (!subscription.m_functionPtr)
		{
			consumed = subscription.m_callback(args);
		}
		else
		{
			consumed = subscription.m_functionPtr(args);
		}
		if (consumed)
			return true;
	}
	return false;
}

bool EventSystem::FireEvent(const std::string& eventName) const
{
    EventArgs args = EventArgs();
    return FireEvent(eventName, args);
}

void EventSystem::GetRegisteredEventNames(std::vector<std::string>& outNames) const
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

    for (const auto& mapEntry : m_subscriptionListMap)
    {
        if (!mapEntry.second.m_list.empty())
        {
            outNames.push_back(mapEntry.first);
        }
    }
}

EventRecipient::EventRecipient(EventSystem& eventSystem)
    : m_eventSystem(eventSystem)
{
}

EventRecipient::~EventRecipient()
{
	m_eventSystem.Unsubscribe(this);
}

EventHandler SubscriptionList::Subscribe(EventCallbackFP callback, Subscriber subscriber)
{
    EventHandler handle = m_nextHandler++;
    m_list.emplace_back(handle, subscriber, callback);
    return handle;
}

EventHandler SubscriptionList::Subscribe(const EventCallback& callback, Subscriber subscriber)
{
	EventHandler handle = m_nextHandler++;
	m_list.emplace_back(handle, subscriber, callback);
	return handle;
}

void SubscriptionList::Unsubscribe(EventCallbackFP fp)
{
	for (auto ite = m_list.begin(); ite != m_list.end(); ++ite)
	{
		if (ite->m_functionPtr == fp)
		{
			m_list.erase(ite);
			return;
		}
	}
}

void SubscriptionList::Unsubscribe(EventHandler handle)
{
	for (auto ite = m_list.begin(); ite != m_list.end(); ++ite)
	{
		if (ite->m_handler == handle)
		{
			m_list.erase(ite);
			return;
		}
	}
}

void SubscriptionList::Unsubscribe(Subscriber subscriber)
{
	for (auto ite = m_list.begin(); ite != m_list.end(); )
	{
		if (ite->m_subscriber == subscriber)
		{
			ite = m_list.erase(ite);
		}
		else
		{
			++ite;
		}
	}
}

bool SubscriptionList::Dispatch(EventArgs& args) const
{
	for (const auto& subscription : m_list)
	{
		bool consumed;
		if (!subscription.m_functionPtr)
		{
			consumed = subscription.m_callback(args);
		}
		else
		{
			consumed = subscription.m_functionPtr(args);
		}
		if (consumed)
			return true;
	}
	return false;
}

