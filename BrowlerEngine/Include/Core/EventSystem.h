#pragma once
class CoreEventSystem;
#include "Common/EventSystem.h"
#include "Events.h"
#include "Renderer/Events.h"

BRWL_NS

class CoreTranslator :
	public Translator<Event>,
	public Translator<RENDERER::Event>
{ };

class CoreEventSystem : public CoreTranslator, public EventSystem<CoreEventSystem, Event, RENDERER::Event>
{
	CoreEventSystem() : EventSystem<CoreEventSystem, Event, RENDERER::Event>(this)
	{ }
};



BRWL_NS_END