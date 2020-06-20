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


template<> template<> bool Translator<Event>::translateMessage<Event::FRAME_IDX_CHANGE>(void* param)
{
	return static_cast<EventBusSwitch<RENDERER::Event>*>(engine->eventSystem.get())->postEventLocal<RENDERER::Event::FRAME_IDX_CHANGE>(param);
}

template<> template<> bool Translator<Event>::translateMessage<Event::SET_FREE_CAM_MOVEMENT>(void* param)
{
	// no need to push to other modules
	return false;
}

template<> template<> bool Translator<Event>::translateMessage<Event::CAM_HAS_MOVED>(void* param)
{
	// Renderer wants to know when the user moved the camera in order to trigger an update to the rendering
	return static_cast<EventBusSwitch<RENDERER::Event>*>(engine->eventSystem.get())->postEventLocal<RENDERER::Event::CAM_HAS_MOVED>(param);
}


BRWL_NS_END