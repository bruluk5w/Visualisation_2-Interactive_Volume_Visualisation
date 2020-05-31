#include "EventSystem.h"
#include "BrowlerEngine.h"
BRWL_NS

template<> template<> bool Translator<Event>::translateMessage<Event::WINDOW_MOVE>(void* param)
{
	using resultTypeA = std::remove_pointer_t<std::invoke_result<decltype(castParam<Event::WINDOW_MOVE>), void*>::type>;
	using resultTypeB = std::remove_pointer_t<std::invoke_result<decltype(castParam<RENDERER::Event::WINDOW_MOVE>), void*>::type>;
	static_assert(alignof(resultTypeA) == alignof(resultTypeB) && std::is_standard_layout_v<resultTypeA> && std::is_standard_layout_v<resultTypeB> && sizeof(resultTypeA) == sizeof(resultTypeB));
	return static_cast<EventBusSwitch<RENDERER::Event>*>(engine->eventSystem.get())->postEventLocal<RENDERER::Event::WINDOW_MOVE>(param);
}

template<> template<> bool Translator<Event>::translateMessage<Event::WINDOW_RESIZE>(void* param)
{
	return static_cast<EventBusSwitch<RENDERER::Event>*>(engine->eventSystem.get())->postEventLocal<RENDERER::Event::WINDOW_RESIZE>(param);
}


BRWL_NS_END