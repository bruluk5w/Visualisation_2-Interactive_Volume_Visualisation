#pragma once // (c) 2020 Lukas Brunner

#include "Core/IUpdatable.h"

BRWL_RENDERER_NS
class Camera;
BRWL_RENDERER_NS_END

BRWL_NS

class Transform;

class Visualization2Updatable : public IUpdatable
{
public:
	Visualization2Updatable();
	// Inherited via IUpdatable
	virtual bool init() override;
	virtual void update(double dt) override;
	virtual void destroy() override;

	std::unique_ptr<RENDERER::Camera> camera;

	bool constrainCam;
	Vec2 constrainedRotation;
	float constrainedRadius;

	size_t windowResizeEvtHdl = 0;
	size_t setCamFreeCamMovementEvtHdl = 0;
};


BRWL_NS_END