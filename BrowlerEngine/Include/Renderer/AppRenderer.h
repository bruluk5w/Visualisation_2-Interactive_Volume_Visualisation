#pragma once

#include "RendererFwd.h"

BRWL_RENDERER_NS


class BaseRenderer;

class AppRenderer
{
	friend class BaseRenderer;
public:
	AppRenderer();

private:
	bool rendererInit();
	void rendererDestroy();

	bool initialized;
public:
	bool isInitalized() { return initialized; }
	virtual bool init() = 0;
	virtual void render(Renderer* renderer) = 0;
	virtual void destroy() = 0;
};


BRWL_RENDERER_NS_END