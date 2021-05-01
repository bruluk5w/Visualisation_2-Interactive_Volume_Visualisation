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

protected:
	//! Called once before preRender() is called for the first time on an uninitialized AppRenderer.
	/**
	 * If initialization does not succeed, destroy(Renderer* renderer) is automatically called.
	 * \return True if initialization succeeded, false if not
	 */
	virtual bool init(Renderer* renderer) = 0;
	//! Called before render() and even before descriptor heaps start a new frame.
	/**
	 * Handle tasks here that don't yet need to manipulate any GPU state but can't be done in parallel to or after
	 * render(Renderer* render).
	 */
	virtual void preRender(Renderer* renderer) = 0;
	//! Called every frame even if no frame may be drawn.
	/**
	 * Manage resources here. Descriptor heaps started a new frame and resource views can be created.
	 * Frames will advance and update resources even if draw is never called.
	 */
	virtual void render(Renderer* renderer) = 0;
	//! Only called if an actual frame is to be drawn and shown to the screen.
	/**
	 * Only implement draw calls here. E.g. when the window  is too small, now draw may be issued.
	 */
	virtual void draw(Renderer* renderer) = 0;
	/**
	 * Destroys any owned or managed resources and reverts any state changed in init.
	 * \param renderer The core renderer that calls this app renderer
	 */
	virtual void destroy(Renderer* renderer) = 0;
};


BRWL_RENDERER_NS_END