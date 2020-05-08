#pragma once

#include <vector>
#include <functional>
#include "Events.h"

BRWL_NS

enum class Event;

class EventBus
{
	using Listener = std::function<bool(Event, void*)>;
	using ListenerArray = std::vector<Listener>;
	using ListenerRegistry = std::vector<ListenerArray>;
public:
	using Handle = size_t;

	EventBus();

	// The listener has to return false if the event should not be consumed but rather be passed on to the next listener and true if the event should be considered consumed
	// The listener takes the event id and an arbitrary void* that can be casted to the proper type inside the callback funktion
	Handle registerListener(Event event, Listener callback);
	bool unregisterListener(Event event, Handle& handle);

	void postEvent(Event event, void* param);
	bool hasAnyListeners();

private:
	ListenerRegistry registry;
};


BRWL_NS_END