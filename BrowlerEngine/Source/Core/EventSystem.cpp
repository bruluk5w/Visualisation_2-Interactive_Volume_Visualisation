#include "EventSystem.h"
#include "BrowlerEngine.h"
BRWL_NS

template<> template<> bool Translator<Event>::translateMessage<Event::WINDOW_MOVE>(void* param)
{
	/*CoreEventSystem* sys = static_cast<CoreEventSystem*>(this);
	EventBusSwitch<Event>* busSwitch = static_cast<EventBusSwitch<Event>*>(sys);
	busSwitch->bus.postEvent(Event::WINDOW_MOVE, param);*/
	//const int a[] = { 0, 1, 2 };
	//const int b[] = { 0, 1, 2 };
	//static_assert(equal(a, b));
	using resultTypeA = std::remove_pointer_t<std::invoke_result<decltype(castParam<Event::WINDOW_MOVE>), void*>::type>;
	using resultTypeB = std::remove_pointer_t<std::invoke_result<decltype(castParam<RENDERER::Event::WINDOW_MOVE>), void*>::type>;
	static_assert(std::is_standard_layout_v<resultTypeA> && std::is_standard_layout_v<resultTypeB> && sizeof(resultTypeA) == sizeof(resultTypeB));
	return static_cast<EventBusSwitch<RENDERER::Event>*>(engine->eventSystem.get())->postEventLocal<RENDERER::Event::WINDOW_MOVE>(param);
}

template<> template<> bool Translator<Event>::translateMessage<Event::WINDOW_RESIZE>(void* param)
{
	return static_cast<EventBusSwitch<RENDERER::Event>*>(engine->eventSystem.get())->postEventLocal<RENDERER::Event::WINDOW_RESIZE>(param);
}


BRWL_NS_END