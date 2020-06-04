#pragma once

#include "Renderer/AppRenderer.h"

BRWL_RENDERER_NS

class BaseRenderer;

class Visualization2Renderer : public AppRenderer
{
public:
	Visualization2Renderer();
protected:
	virtual bool init() override;
	virtual void render(PAL::WinRenderer* renderer) override;
	virtual void destroy() override;
};

BRWL_RENDERER_NS_END