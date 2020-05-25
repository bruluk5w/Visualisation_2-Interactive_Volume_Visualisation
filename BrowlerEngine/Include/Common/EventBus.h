#pragma once

#include "Events.h"

#include <vector>
#include <functional>
#include <algorithm>

BRWL_NS

template<typename Event>
class EventBus
{
	static_assert(Utils::is_enum_class<Event>::value);
public:
	using Listener = std::function<bool(Event, void*)>;
protected:
	using ListenerArray = std::vector<Listener>;
	using ListenerRegistry = std::vector<ListenerArray>;
	using EventBusT = EventBus<Event>;
public:
	using Handle = size_t;

	EventBus() : registry(ENUM_CLASS_TO_NUM(Event::MAX))
	{ }

	// The listener has to return false if the event should not be consumed but rather be passed on to the next listener and true if the event should be considered consumed
	// The listener takes the event id and an arbitrary void* that can be casted to the proper type inside the callback funktion
	Handle registerListener(Event event, Listener callback)
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

	bool unregisterListener(Event event, Handle& handle)
	{
		ListenerArray& listeners = registry[ENUM_CLASS_TO_NUM(event)];
		if (handle < listeners.size() && listeners[handle] != nullptr)
		{
			listeners[handle] = nullptr;
			// maybe there is a better value, since 0 could be a handle
			handle = 0;
			return true;
		}

		// maybe there is a better value, since 0 could be a handle
		handle = 0;
		return false;
	}

	bool postEvent(Event event, void* param)
	{
		ListenerArray& listeners = registry[ENUM_CLASS_TO_NUM(event)];
		for (Listener& listener : listeners)
		{
			// if consumed, then we break
			if (listener != nullptr && listener(event, param)) return true;
		}

		return false;
	}

	bool hasAnyListeners()
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

private:
	ListenerRegistry registry;
};


BRWL_NS_END