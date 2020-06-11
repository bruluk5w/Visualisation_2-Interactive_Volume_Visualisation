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
	rotX(0),
	rotY(PI_F)
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
	camera->rotation() = Quaternion(right, 180.f * DEG_2_RAD_F);
	cameraPawn = std::make_unique<Transform>();
	camera->setParent(cameraPawn.get());
	engine->hierarchy->addToRoot(cameraPawn.get());
	cameraPawn->position() = { 0.f, 0.5f, 3.f };

	engine->renderer->setCamera(camera.get());

	return true;
}

void Visualization2Updatable::update(double dt)
{
	////const Quaternion& globalRot = camera->getGlobalOrientation();
	//const Vec3 forward = cameraPawn->forward();
	//const Vec3 pos = camera->getGlobalPosition();

	
	//const Vec3 right = cameraPawn->right();
	//const Vec3 up = cameraPawn->up();
	Vec3 strave{ 0.f, 0.f, 0.f };
	
	if (engine->input->isKeyPressed(Key::W)) {
		strave.z -= 1;
	}

	if (engine->input->isKeyPressed(Key::S)) {
		strave.z += 1;
	}

	if (engine->input->isKeyPressed(Key::D)) {
		strave.x += 1;
	}

	if (engine->input->isKeyPressed(Key::A)) {
		strave.x -= 1;
	}

	if (engine->input->isKeyPressed(Key::Q)) {
		strave.y -= 1;
	}

	if (engine->input->isKeyPressed(Key::E)) {
		strave.y += 1;
	}

	if (strave.x || strave.y || strave.z) {
		const float speed = engine->input->isKeyPressed(Key::SHIFT) ? 3.5f : 1.5f;
		cameraPawn->position() += cameraPawn->rotation() * normalize(strave) * speed * dt;
	}

	////const InputManager* i = engine->input.get();
	////float mouseX = i->getMouseDeltaX();
	////float mouseY = i->getMouseDeltaY();
	////if (i->isKeyPressed(Key::ALT))
	////{
	////	if (i->isButtonDown(Button::MOUSE_1))
	////	{
	////		Vec3 worldRay = screenSpaceToWorldRay(i->getMouseX(), i->getMouseY(), *camera);
	////		if (worldRay.y == 0 || pos.y == 0 || std::abs(pos.y) - 0.02 <= std::abs(worldRay.y + pos.y))
	////		{
	////			return;
	////		}
	////		// intersect with plane y = 0;
	////		worldRay *= pos.y / worldRay.y;
	////		orbitOrigin->position() = Vec3(worldRay.x, 0.f, worldRay.z);
	////		BRWL_CHAR buf[100];
	////		BRWL_SNPRINTF(buf, countof(buf), BRWL_CHAR_LITERAL("Pivot Set: %.2f/%.2f/%.2f"), orbitOrigin->cPosition().x, orbitOrigin->cPosition().y, orbitOrigin->cPosition().z);
	////		engine->logger->info(buf);
	////	}

	////	if (i->isButtonPressed(Button::MOUSE_1) && (mouseX != 0.0f || mouseY != 0.0f))
	////	{
	////		Vec3 offset = (cameraPawn->cPosition() - orbitOrigin->cPosition());
	////		Vec3 newOffset = Quaternion(up, -mouseX * DEG_2_RAD_F * 0.1f) * (Quaternion(right, mouseY * DEG_2_RAD_F * 0.1f) * offset);
	////		cameraPawn->position() = orbitOrigin->cPosition() + newOffset;
	////		//camera->rotation() = Quaternion(up, 90.f * DEG_2_RAD_F) * Quaternion::lookAt(camera->cPosition(), orbitOrigin->cPosition());

	////		BRWL_CHAR buf[500];
	////		BRWL_SNPRINTF(buf, countof(buf), BRWL_CHAR_LITERAL("%.2f/%.2f/%.2f"), forward.x * RAD_2_DEG_F, forward.y * RAD_2_DEG_F, forward.z * RAD_2_DEG_F);
	////		//BRWL_SNPRINTF(buf, countof(buf), BRWL_CHAR_LITERAL("Cam pos:%.2f/%.2f/%.2f"), cameraPawn->position().x, cameraPawn->position().y, cameraPawn->position().z);
	////		engine->logger->info(buf);
	////	}
	////}

	float dx = engine->input->getMouseDeltaX();
	float dy = engine->input->getMouseDeltaY();
	if (engine->input->isButtonPressed(Button::MOUSE_1) && (dx != 0 || dy != 0))
	{
		//rotX = Utils::clamp(rotX + dy * DEG_2_RAD_F * 0.1f, -PI_F + 0.1f, PI_F - 0.1f);
		//rotY -= dx * DEG_2_RAD_F * 0.1f;
		//cameraPawn->rotation() = Quaternion::makeFromEuler({ rotX, rotY, 0.0f });

		{
			Vec3 euler = cameraPawn->rotation().toEuler();
			euler.y -= dx / 300.0f;
			euler.x += dy / 300.0f;

			// clamp ath 90 degrees x rotation
			euler.x = euler.x > 0.5f * PI_F ? 0.5f * PI_F : euler.x < -0.5f * PI_F ? -0.5f * PI_F : euler.x;

			cameraPawn->rotation().fromEuler(euler.x, euler.y, euler.z);
		}
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