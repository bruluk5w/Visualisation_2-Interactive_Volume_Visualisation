#include "AppRenderer.h"

BRWL_RENDERER_NS


AppRenderer::AppRenderer() :
	initialized(false)
{ }

bool AppRenderer::rendererInit(Renderer* renderer)
{
	if (!initialized)
	{
		initialized = init(renderer);
		if (!initialized)
		{
			destroy(renderer);
		}
	}

	return initialized;
}

void AppRenderer::rendererDestroy(Renderer* renderer)
{
	if (initialized)
	{
		initialized = false;
		destroy(renderer);
	}
}


BRWL_RENDERER_NS_END