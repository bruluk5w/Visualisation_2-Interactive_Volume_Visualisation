#pragma once

#include "Common/EventBus.h"

BRWL_NS


template<typename EventType, typename ChildEventSystem>
class EventBusSwitch {
	static_assert(Utils::is_enum_class<EventType>::value);
	static_assert(std::is_base_of_v<EventBusSwitch<EventType, ChildEventSystem>, ChildEventSystem>);
public:
	template<EventType event>
	void postEvent(void* param) {
		bus.postEvent(event, param);
		static_cast<ChildEventSystem*>(this)->translateMessage<event>(param);
	}

	// This function can be used to tie multiple events which mean the same thing from different modules toghether.
	// E.g.: A window resizing event is produced by the Core module and consumed by the Renderer module. However the
	// the Core module depends on the Renderer module and hence the Renderer cannot consume the resizing event of the 
	// Core module directly. It doesn't know its definition and hence defines its own resizing event.
	// The event system of the Core module has to arrange for the resizing event of the Renderer module being fired
	// accordingly upon dispatch of the resizing event of the Core module.
	// Ideally the event system of the Core module also takes care of firing it's own resizing event when the resizing
	// event is dispatched using the event definition from the Renderer module.
	template<EventType event>
	void translateMessage(void* param) { /* This is empty, so it should be optimized away for all events that don't have a sibling counterpart in another module */ };
private:
	EventBus<EventType> bus;
};

template<typename... EventTypes>
class EventSystem : public EventBusSwitch<EventTypes, EventSystem<EventTypes>>... {
public:
	//foo(Interfaces... ifaces) : Interfaces(ifaces)... {}

};

BRWL_NS_END