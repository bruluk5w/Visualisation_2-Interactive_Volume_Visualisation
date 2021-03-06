#pragma once // (c) 2020 Lukas Brunner

#include "Common/EventSystem.h"
#include "Events.h"
#include "Renderer/Events.h"

BRWL_NS


using CoreEventSystem = EventSystem<Event, RENDERER::Event>;


BRWL_NS_END