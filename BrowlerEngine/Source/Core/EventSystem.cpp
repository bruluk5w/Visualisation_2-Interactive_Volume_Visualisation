#include "EventSystem.h"

BRWL_NS

template<> template<> void Translator<Event>::translateMessage<Event::WINDOW_MOVE>(void* param)
{
	CoreEventSystem* sys = static_cast<CoreEventSystem*>(this);
	EventBusSwitch<Event>* busSwitch = static_cast<EventBusSwitch<Event>*>(sys);
	busSwitch->bus.postEvent(Event::WINDOW_MOVE, param);
	BRWL_CHECK(false, BRWL_CHAR_LITERAL("Translate!"));
}

template<> template<> void Translator<Event>::translateMessage<Event::WINDOW_RESIZE>(void* param)
{
	BRWL_CHECK(false, BRWL_CHAR_LITERAL("Translate!"));
}
//template<> EventBusSwitch<RENDERER::Event>::BusT& Translator<Event>::getBus<RENDERER::Event>() {
//	
//}

BRWL_NS_END