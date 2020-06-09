#pragma once // (c) 2020 Lukas Brunner

#include "RendererFwd.h"

BRWL_RENDERER_NS


class BaseRenderer;

class AppRenderer
{
	friend class BaseRenderer;
	friend typedef Renderer;
public:
	AppRenderer();

private:
	bool rendererInit(Renderer* renderer);
	void rendererDestroy(Renderer*);

	bool initialized;
public:
	bool isInitalized() { return initialized; }
	virtual bool init(PAL::WinRenderer* renderer) = 0;
	virtual void preRender(PAL::WinRenderer* renderer) = 0;
	virtual void render(PAL::WinRenderer* renderer) = 0;
	virtual void draw(Renderer* renderer) = 0;
	virtual void destroy(Renderer* renderer) = 0;
};


BRWL_RENDERER_NS_END