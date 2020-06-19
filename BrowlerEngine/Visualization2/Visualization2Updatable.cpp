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
	camera(nullptr),
	constrainCam(true)
{ }

bool Visualization2Updatable::init()
{
	int width = engine->window->width();
	int height = engine->window->height();

	camera = std::make_unique<RENDERER::Camera>(width, height, 45.f * DEG_2_RAD_F, 0.1f, 500.f, BRWL_CHAR_LITERAL("Main Camera"));
	camera->position() = { 0.f, 0.0f, -3.f };
	engine->hierarchy->addToRoot(camera.get());
	engine->renderer->setCamera(camera.get());
	
	auto* coreBus = static_cast<EventBusSwitch<Event>*>(engine->eventSystem.get());
	windowResizeEvtHdl = coreBus->registerListener(Event::WINDOW_RESIZE, [this](Event, void* param) {
		if (camera) {
			auto* p = castParam<Event::WINDOW_RESIZE>(param);
			camera->resize(p->width, p->height);
		}

		return false;
	});

	setCamFreeCamMovementEvtHdl = coreBus->registerListener(Event::SET_FREE_CAM_MOVEMENT, [this](Event, void* param) {
		onConstrainCam(!castParam<Event::SET_FREE_CAM_MOVEMENT>(param)->value);
		return false;
	});
	onConstrainCam(constrainCam);

	return true;
}

void Visualization2Updatable::update(double dt)
{
	Vec3 direction{ 0.f, 0.f, 0.f };

	if (engine->input->isKeyPressed(Key::W)) {
		direction.z += 1;
	}

	if (engine->input->isKeyPressed(Key::S)) {
		direction.z -= 1;
	}

	if (engine->input->isKeyPressed(Key::D)) {
		direction.x += 1;
	}

	if (engine->input->isKeyPressed(Key::A)) {
		direction.x -= 1;
	}

	if (engine->input->isKeyPressed(Key::E)) {
		direction.y += 1;
	}

	if (engine->input->isKeyPressed(Key::Q)) {
		direction.y -= 1;
	}

	float dx = (float)engine->input->getMouseDeltaX();
	float dy = (float)engine->input->getMouseDeltaY();

	const bool hasMouseAction = engine->input->isButtonPressed(Button::MOUSE_1) && (dx != 0 || dy != 0);
	const bool hasKeyAction = direction.x || direction.y || direction.z;
	if (hasMouseAction || hasKeyAction)
	{
		const float speed = engine->input->isKeyPressed(Key::SHIFT) ? 4.5f : 1.5f;
		if (constrainCam)
		{
			constrainedRadius = Utils::max(constrainedRadius + direction.y * speed * (float)dt, 0.01f);
		
			const float rotationMultiplier = 1.f / 15.f;
			float rotY = dt * (-direction.x * 0.7f  + (hasMouseAction ? dx * rotationMultiplier : 0.f)) * speed;
			float rotX = dt * (direction.z * 0.7f + (hasMouseAction ? dy * rotationMultiplier : 0.f)) * speed;

			// apply a maximum delta per frame in case we have a super low frame rate
			constrainedRotation.x += Utils::clamp(rotX, -PI_F * 0.0625f, PI_F * 0.0625f);
			// clamp to +-90°
			constrainedRotation.x = Utils::clamp(constrainedRotation.x, 0.5f * PI_F - 0.0001f, -0.5f * PI_F + 0.0001f);
			constrainedRotation.y += Utils::clamp(rotY, -PI_F * 0.125f, PI_F * 0.125f);
			camera->rotation().fromEuler(constrainedRotation.x, constrainedRotation.y, 0);

			camera->position() = camera->rotation() * -VEC3_FWD * constrainedRadius;
		}
		else {

			if (hasMouseAction)
			{
				const float rotationMultiplier = 1.f / 300.f;
				Vec3 euler = camera->rotation().toEuler();
				euler.y += dx * rotationMultiplier;
				euler.x += dy * rotationMultiplier;

				// clamp ath 90 degrees x rotation
				euler.x = euler.x > 0.5f * PI_F ? 0.5f * PI_F : euler.x < -0.5f * PI_F ? -0.5f * PI_F : euler.x;
				camera->rotation().fromEuler(euler.x, euler.y, euler.z);
			}

			if (hasKeyAction) {
				camera->position() += camera->rotation() * normalized(direction) * speed * dt;
			}
		}
	}
}

void Visualization2Updatable::destroy()
{
	if (engine->eventSystem)
	{
		auto* coreBus = static_cast<EventBusSwitch<Event>*>(engine->eventSystem.get());
		coreBus->unregisterListener(Event::WINDOW_RESIZE, windowResizeEvtHdl);
		coreBus->unregisterListener(Event::SET_FREE_CAM_MOVEMENT, setCamFreeCamMovementEvtHdl);
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

void Visualization2Updatable::onConstrainCam(bool enable)
{
	constrainCam = enable;
	if (constrainCam)
	{
		camera->rotation() = Quaternion::fromTo(VEC3_FWD, -camera->cPosition());
		constrainedRadius = length(camera->cPosition());
		Vec3 dir = normalized(camera->cPosition());
		constrainedRotation.x = sin(dir.y);
		constrainedRotation.y = atan2(dir.x, dir.z) - PI_F;
		//constrainedRotation.x = Utils::clamp(constrainedRotation.x, 0.5f * PI_F - 0.0001f, -0.5f * PI_F + 0.0001f);
		camera->rotation().fromEuler(constrainedRotation.x, constrainedRotation.y, 0);
	}
}

BRWL_NS_END