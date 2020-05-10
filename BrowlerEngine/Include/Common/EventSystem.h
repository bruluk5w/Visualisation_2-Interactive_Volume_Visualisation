#pragma once

#include "Common/EventBus.h"

BRWL_NS
template<typename EventType>
class EventSystem {
public:
	void postEvent(EventType event, void* param) {
		bus.postEvent(event, param);
	}
private:
	EventBus<EventType> bus;
};

BRWL_NS_END