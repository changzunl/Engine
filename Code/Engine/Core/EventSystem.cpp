#include "EventSystem.hpp"

EventSystem* g_theEventSystem = nullptr;

EventSubscription::EventSubscription(const EventCallbackFunction& functionPtr)
    : m_functionPtr(functionPtr)
{
}

EventSubscription::~EventSubscription()
{

}

EventSystem::EventSystem(const EventSystemConfig& theConfig)
    : m_theConfig(theConfig)
{
}

EventSystem::~EventSystem()
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

void EventSystem::SubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFunction functionPtr)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

    m_subscriptionListMap[eventName].emplace_back(functionPtr);
}

void EventSystem::UnsubscribeEventCallbackFunction(const std::string& eventName, EventCallbackFunction functionPtr)
{
	std::lock_guard<std::mutex> guard(m_subscriptionMutex);

    EventSubscriptionMap::iterator iter = m_subscriptionListMap.find(eventName);
    if (iter != m_subscriptionListMap.end())
    {
        SubscriptionList& list = iter->second;
        for (SubscriptionList::iterator subIter = list.begin(); subIter != list.end(); subIter++)
        {
            if (subIter->m_functionPtr == functionPtr)
            {
                list.erase(subIter);
                return;
            }
        }
    }
}

bool EventSystem::FireEvent(const std::string& eventName, EventArgs& args) const
{
    SubscriptionList list;
	{
		std::lock_guard<std::mutex> guard(m_subscriptionMutex);

		EventSubscriptionMap::const_iterator iter = m_subscriptionListMap.find(eventName);
		if (iter != m_subscriptionListMap.end())
			list.insert(list.end(), iter->second.begin(), iter->second.end());
    }
	for (const auto& subscription : list)
	{
		bool consumed = subscription.m_functionPtr(args);
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
        if (mapEntry.second.size() > 0)
        {
            outNames.push_back(mapEntry.first);
        }
    }
}

