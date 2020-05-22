#pragma once

#include "Common/EventBus.h"

BRWL_NS
template<typename EventType>
class EventBusSwitch;

// This class should never be instantiated but only derived once from.
// Specialized implementations for the template methods in this class should downcast to the derived type which should have access to the EventBusSwitches of the other modules.
// The derived type can then pass the event on to other modules.
template<typename EventType>
struct Translator {
	// This function can be used to tie multiple events which mean the same thing from different modules toghether.
	// E.g.: A window resizing event is produced by the Core module and consumed by the Renderer module. However the
	// the Core module depends on the Renderer module and hence the Renderer cannot consume the resizing event of the 
	// Core module directly. It doesn't know its definition and hence defines its own resizing event.
	// The event system of the Core module has to arrange for the resizing event of the Renderer module being fired
	// accordingly upon dispatch of the resizing event of the Core module.
	// Ideally the event system of the Core module also takes care of firing it's own resizing event when the resizing
	// event is dispatched using the event definition from the Renderer module.
	template<EventType event>
	static bool translateMessage(void* param);// { BRWL_CHECK(false, BRWL_CHAR_LITERAL("boom"));/* This is empty, so it should be optimized away for all events that don't have a sibling counterpart in another module */ };
private:
	//// this has to be specialized in order to allow one Translator for events from module A to access the plain event bus from a module B
	//template<typename ForeignEventType>
	//typename EventBusSwitch<ForeignEventType>::BusT& getBus();/* {
	//	// fail on instantiation
	//	static_assert(std::false_type && std::is_same_v<EventType, int>, "You need to implement this version of getBus.");
	//}*/
};

template<typename EventType>
class EventBusSwitch {
	static_assert(Utils::is_enum_class<EventType>::value);
	//static_assert(std::is_base_of_v<EventBusSwitch<EventType, ChildEventSystem>, ChildEventSystem>);

	typedef EventBus<EventType> BusT;

public:
	EventBusSwitch()
	{ }

	// Send the event to all listeners also in other modules
	template<EventType event>
	bool postEvent(void* param) {
		if (!bus.postEvent(event, param))
		{
			// if not consumed in the local module, then continue sending it to others
			return Translator<EventType>::translateMessage<event>(param);
		}

		return true;
	}

	// Only post the event to all listeners in the local module
	template<EventType event>
	bool postEventLocal(void* param) {
		return bus.postEvent(event, param);
	}

	typename BusT::Handle registerListener(EventType event, typename BusT::Listener callback)
	{
		return bus.registerListener(event, callback);
	}


	bool unregisterListener(EventType event, typename BusT::Handle& handle)
	{
		return bus.unregisterListener(event, handle);
	}

	bool hasAnyListeners()
	{
		return bus.hasAnyListeners();
	}

	
protected:

	BusT bus;
	Translator<EventType>* translator;
};

template<typename... EventTypes>
class EventSystem : public EventBusSwitch<EventTypes>... {
public:
	EventSystem() : EventBusSwitch<EventTypes>()... { }

	bool hasAnyListeners()
	{
		return (((EventBusSwitch<EventTypes>*)this)->hasAnyListeners() && ...);
	}
};

BRWL_NS_END