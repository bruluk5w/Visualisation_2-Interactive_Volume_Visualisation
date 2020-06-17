#include "Visualization2Updatable.h"

#include "Common/Transform.h"
#include "Renderer/Camera.h"
#include "Core/BrowlerEngine.h"
#include "Renderer/Renderer.h"
#include "Core/Window.h"
#include "Core/Hierarchy.h"
#include "Core/Input.h"
#include "Common/Logger.h"

BRWL_NS


Visualization2Updatable::Visualization2Updatable() :
	IUpdatable(),
	camera(nullptr)
{ }

bool Visualization2Updatable::init()
{
	int width = engine->window->width();
	int height = engine->window->height();
	
	auto* coreBus = static_cast<EventBusSwitch<Event>*>(engine->eventSystem.get());
	windowResizeEvtHdl = coreBus->registerListener(Event::WINDOW_RESIZE, [this](Event, void* param) {
		if (camera) {
			auto* p = castParam<Event::WINDOW_RESIZE>(param);
			camera->resize(p->width, p->height);
		}

		return false;
	});

	camera = std::make_unique<RENDERER::Camera>(width, height, 45.f * DEG_2_RAD_F, 0.1f, 500.f, BRWL_CHAR_LITERAL("Main Camera"));
	camera->position() = { 0.f, 0.0f, -3.f };
	engine->hierarchy->addToRoot(camera.get());
	engine->renderer->setCamera(camera.get());

	return true;
}

void Visualization2Updatable::update(double dt)
{
	Vec3 strave{ 0.f, 0.f, 0.f };

	if (engine->input->isKeyPressed(Key::W)) {
		strave.z += 1;
	}

	if (engine->input->isKeyPressed(Key::S)) {
		strave.z -= 1;
	}

	if (engine->input->isKeyPressed(Key::D)) {
		strave.x += 1;
	}

	if (engine->input->isKeyPressed(Key::A)) {
		strave.x -= 1;
	}

	if (engine->input->isKeyPressed(Key::E)) {
		strave.y += 1;
	}

	if (engine->input->isKeyPressed(Key::Q)) {
		strave.y -= 1;
	}

	if (strave.x || strave.y || strave.z) {
		const float speed = engine->input->isKeyPressed(Key::SHIFT) ? 3.5f : 1.5f;
		camera->position() += camera->rotation() * normalized(strave) * speed * dt;
	}

	float dx = (float)engine->input->getMouseDeltaX();
	float dy = (float)engine->input->getMouseDeltaY();
	if (engine->input->isButtonPressed(Button::MOUSE_1) && (dx != 0 || dy != 0))
	{
		Vec3 euler = camera->rotation().toEuler();
		euler.y += dx / 300.0f;
		euler.x += dy / 300.0f;

		// clamp ath 90 degrees x rotation
		euler.x = euler.x > 0.5f * PI_F ? 0.5f * PI_F : euler.x < -0.5f * PI_F ? -0.5f * PI_F : euler.x;
		camera->rotation().fromEuler(euler.x, euler.y, euler.z);
	}
}

void Visualization2Updatable::destroy()
{
	if (engine->eventSystem)
	{
		auto* coreBus = static_cast<EventBusSwitch<Event>*>(engine->eventSystem.get());
		coreBus->unregisterListener(Event::WINDOW_RESIZE, windowResizeEvtHdl);
		windowResizeEvtHdl = 0;
	}

	if (camera)
	{
		camera->setParent(nullptr);

		if (engine->renderer && engine->renderer->getCamera() == camera.get())
		{
			engine->renderer->setCamera(nullptr);
		}

		camera = nullptr;
	}
}


BRWL_NS_END