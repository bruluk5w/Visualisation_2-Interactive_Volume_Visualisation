#pragma once

#include "Common/EventSystem.h"
#include "Events.h"
#include "Renderer/Events.h"

BRWL_NS


//class CoreEventSystem : public EventSystem<Event, CoreEventSystem>, public EventSystem <RENDERER::Event, CoreEventSystem> {
//	template<Event::WINDOW_RESIZE>
//	void translateMessage(void* param) {
//		(EventSystem <RENDERER::Event, CoreEventSystem>)
//	};
//};

using CoreEventSystem = EventSystem<Event, RENDERER::Event>;


BRWL_NS_END