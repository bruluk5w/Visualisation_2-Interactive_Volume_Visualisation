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

	size_t windowResizeEvtHdl = 0;
	float rotY;
	float rotX;
	std::unique_ptr<Transform> cameraPawn;
};


BRWL_NS_END