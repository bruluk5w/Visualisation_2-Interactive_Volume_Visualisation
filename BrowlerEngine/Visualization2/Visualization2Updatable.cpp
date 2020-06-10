#include "Visualization2Updatable.h"

#include "Renderer/Camera.h"
#include "Core/BrowlerEngine.h"
#include "Renderer/Renderer.h"
#include "Core/Window.h"
#include "Core/Hierarchy.h"
#include "Core/Input.h"

BRWL_NS


Visualization2Updatable::Visualization2Updatable() :
	IUpdatable(),
	camera(nullptr),
	rotX(0),
	rotY(0)
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

	camera = std::make_unique<RENDERER::Camera>(width, height, 70.f, 0.1f, 500.f, BRWL_CHAR_LITERAL("Main Camera"));
	engine->hierarchy->addToRoot(camera.get());
	engine->renderer->setCamera(camera.get());

	return true;
}

void Visualization2Updatable::update(double dt)
{
	Vec3 strive{ 0.f, 0.f, 0.f };
	const Vec3 forward = camera->forward();
	const Vec3 right = camera->right();
	const Vec3 up = camera->up();
	
	if (engine->input->isKeyPressed(Key::W)) {
		strive += forward;
	}

	if (engine->input->isKeyPressed(Key::S)) {
		strive -= forward;
	}

	if (engine->input->isKeyPressed(Key::D)) {
		strive += right;
	}

	if (engine->input->isKeyPressed(Key::A)) {
		strive -= right;
	}

	if (engine->input->isKeyPressed(Key::Q)) {
		strive += up;
	}

	if (engine->input->isKeyPressed(Key::E)) {
		strive -= up;
	}

	camera->position() += strive * (float)dt;
	float mouseX =  engine->input->getMouseDeltaX();
	float mouseY =  engine->input->getMouseDeltaY();
	if (engine->input->isMouseButtonPressed(Button::MOUSE_1))
	{
		rotY += mouseX * dt * 0.02f;
		rotX += mouseY * dt * 0.02f;
		camera->rotation() = Quaternion::fromAxisAngle({ 0.f,1.0f,0.f }, rotY) * Quaternion::fromAxisAngle({ 1.f,0.0f,0.f }, rotX);
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

	camera->setParent(nullptr);

	if (engine->renderer && engine->renderer->getCamera() == camera.get())
	{
		engine->renderer->setCamera(nullptr);
	}
}


BRWL_NS_END