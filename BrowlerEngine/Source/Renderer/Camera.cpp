#include "Camera.h"

BRWL_RENDERER_NS


Camera::Camera(int viewportWidth, int viewportHeight, float fovY, float near, float far, const BRWL_STR name) :
	Transform(std::move(name)),
	fovY(fovY),
	near(near),
	far(far)
{
	resize(viewportWidth, viewportHeight);
}

void Camera::resize(int viewportWidth, int viewportHeight)
{
	width = viewportWidth;
	height = viewportHeight;
	projectionDirty = true;
}

void Camera::updateMatrices() const
{
	uint64_t hash = cameraHash();
	if (lastCameraHash != hash)
	{
		inverseDirty = projectionDirty = true;
		modelMatrix = makeAffineTransform(pos, rot, scale);
		const Transform* t = this;
		while (t->getConstParent()) {
			t = t->getConstParent();
			modelMatrix = makeAffineTransform(t->cPosition(), t->cRotation(), t->cScaling()) * modelMatrix;
		}

		viewMatrix = inverse(modelMatrix);// glm::inverse(MatrixUtils::makeAffineTransform(pos, rot));
		inverseViewMatrix = modelMatrix;
		lastCameraHash = hash;
	}

	if (projectionDirty)
	{
		projectionMatrix = makePerspective(fovY, (float)width / (float)height, near, far);
		inverseProjectionMatrix = inverse(projectionMatrix);
		viewProjectionMatrix = projectionMatrix * viewMatrix;
		projectionDirty = false;
	}
}

void Camera::updateInverseMatrix() const {
	updateMatrices();
	if (inverseDirty) {
		inverseViewProjectionMatrix = inverse(viewProjectionMatrix);
		inverseDirty = false;
	}
}


BRWL_RENDERER_NS_END