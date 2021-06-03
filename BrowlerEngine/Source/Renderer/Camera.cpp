#include "Camera.h"

BRWL_RENDERER_NS


Camera::Camera(int viewportWidth, int viewportHeight, float fovY, float near, float far, const BRWL_CHAR* name) :
	Transform(name),
	fovY(fovY),
	near(near),
	far(far)
{
	resize(viewportWidth, viewportHeight);
}

void Camera::resize(int viewportWidth, int viewportHeight)
{
	width = Utils::max(1, viewportWidth);
	height = Utils::max(1, viewportHeight);
	projectionDirty = true;
}

Vec2 Camera::getVisibleRectangle(float distance) const
{

	float height = 2.0f * std::tan(fovY * 0.5f) * distance;
	return { height * aspect(), height };
}

bool Camera::updateMatrices(bool force)
{
	if (force)
	{
		uint64_t hash = cameraHash();
		if (lastCameraHash == hash) return false;
		lastCameraHash = hash;
	}
	else
	{
		uint64_t hash = weakCameraHash();
		if (lastCameraHash == hash) return false;
	}

	inverseProjectionDirty = projectionDirty = true;

	if (force)
	{	// only update projection, not global position in case force == false
		localMatrix = makeAffineTransform(pos, rot, scale);
		Transform* t = this;
		while (t->getParent()) {
			t = t->getParent();
			t->localMatrix = makeAffineTransform(t->cPosition(), t->cRotation(), t->cScaling());
			modelMatrix = t->localMatrix * modelMatrix;
		}

		t->modelMatrix = t->localMatrix;
	}

	viewMatrix = inverse(modelMatrix);
	inverseViewMatrix = modelMatrix;

	if (projectionDirty)
	{
		projectionMatrix = makePerspective(fovY, aspect(), near, far);
		inverseProjectionMatrix = inverse(projectionMatrix);
		viewProjectionMatrix = viewMatrix * projectionMatrix;
		projectionDirty = false;
	}

	return true;
}

void Camera::updateInverseMatrix(bool force) {
	updateMatrices(force);

	if (inverseProjectionDirty)
	{
		inverseViewProjectionMatrix = inverse(viewProjectionMatrix);
		inverseProjectionDirty = false;
	}
}


BRWL_RENDERER_NS_END