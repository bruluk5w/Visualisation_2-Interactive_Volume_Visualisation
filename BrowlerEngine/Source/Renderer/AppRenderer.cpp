#include "AppRenderer.h"

BRWL_RENDERER_NS


AppRenderer::AppRenderer() :
	initialized(false)
{ }

bool AppRenderer::rendererInit()
{
	if (!initialized)
	{
		initialized = init();
		if (!initialized)
		{
			destroy();
		}
	}

	return initialized;
}

void AppRenderer::rendererDestroy()
{
	if (initialized)
	{
		initialized = false;
		destroy();
	}
}


BRWL_RENDERER_NS_END