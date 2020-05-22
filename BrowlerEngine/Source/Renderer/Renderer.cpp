#include "Renderer.h"

BRWL_RENDERER_NS


Renderer::Renderer(EventBusSwitch<Event>* eventSystem) :
	eventSystem(eventSystem),
	windowResizeEventHandle(0)
{
	windowResizeEventHandle = eventSystem->registerListener(Event::WINDOW_RESIZE, [](Event, void* param) -> bool {BRWL_CHECK(false, L"Resize");  return false; });
}

Renderer::~Renderer()
{ }

bool Renderer::init()
{
	return true;
}

void Renderer::destroy()
{
	bool success = eventSystem->unregisterListener(Event::WINDOW_RESIZE, windowResizeEventHandle);
	BRWL_CHECK(success, BRWL_CHAR_LITERAL("Failed to unregister a listener!"));
}



BRWL_RENDERER_NS_END
