#include "EventBus.h"
#include "Events.h"
#include <algorithm>


BRWL_NS


EventBus::EventBus() : registry(ENUM_CLASS_TO_NUM(Event::MAX))
{ }

EventBus::Handle EventBus::registerListener(Event event, Listener callback)
{
	ListenerArray& listeners = registry[ENUM_CLASS_TO_NUM(event)];
	// find a free spot where we can add the listener
	auto it = std::find(listeners.begin(), listeners.end(), nullptr);
	if (it == listeners.end())
	{
		listeners.emplace_back(std::move(callback));
		return listeners.size() - 1;
	}
	else
	{
		*it = callback;
		// returns the index to the callbackfunction as a handle
		return it - listeners.begin();
	}
}

bool EventBus::unregisterListener(Event event, Handle& handle)
{
	ListenerArray& listeners = registry[ENUM_CLASS_TO_NUM(event)];
	if (handle < listeners.size() && listeners[handle] != nullptr)
	{
		listeners[handle] = nullptr;
		return true;
	}

	// maybe there is a better value, since 0 could be a handle
	handle = 0;
	return false;
}

void EventBus::postEvent(Event event, void* param)
{
	ListenerArray& listeners = registry[ENUM_CLASS_TO_NUM(event)];
	for (Listener& listener : listeners)
	{
		// if consumed, then we break
		if (listener != nullptr && listener(event, param)) break;
	}
}

bool EventBus::hasAnyListeners()
{
	for (const ListenerArray& listeners : registry)
	{
		if (!listeners.empty() && std::find_if(listeners.cbegin(), listeners.cend(), [](const Listener& l) -> bool { return l != nullptr; }) != listeners.cend())
		{
			return true;
		}
	}

	return false;
}


BRWL_NS_END