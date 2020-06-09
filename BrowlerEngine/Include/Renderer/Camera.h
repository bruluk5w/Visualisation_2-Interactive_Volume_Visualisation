#pragma once

#include "Common/Transform.h"

BRWL_RENDERER_NS


class Camera : public Transform {
public:
	Camera(int viewportWidth, int viewportHeight, float fovY, float near, float far, BRWL_STR name = BRWL_CHAR_LITERAL("Camera"));

	void resize(int viewportWidth, int viewportHeight);

	void setHorizontalFov(float fovY) {
		this->fovY = fovY;
		inverseDirty = projectionDirty = true;
	}

	void setNearPlane(float zDepth) {
		near = zDepth;
		inverseDirty = projectionDirty = true;
	}

	void setFarPlane(float zDepth) {
		far = zDepth;
		inverseDirty = projectionDirty = true;
	}

	void setNearAndFarPlanes(float near, float far) {
		this->near = near;
		this->far = far;
		inverseDirty = projectionDirty = true;
	}

	float getNearPlane() const { return near; }

	float getFarPlane() const { return far; }

	const Mat4& getViewMatrix(bool forceUpdate = false) { if (forceUpdate) updateMatrices(); return viewMatrix; }
	Mat4 getViewMatrixCopy() { updateMatrices(); return viewMatrix; }

	const Mat4& getProjectionMatrix(bool forceUpdate = false) const { if (forceUpdate) updateMatrices(); return projectionMatrix; }
	Mat4 getProjectionMatrixCopy(bool forceUpdate = false) const { if (forceUpdate) updateMatrices(); return projectionMatrix; }

	const Mat4& getViewProjectionMatrix(bool forceUpdate = false) const { if (forceUpdate) updateMatrices(); return viewProjectionMatrix; }
	Mat4 getViewProjectionMatrixCopy(bool forceUpdate = false) const { if (forceUpdate) updateMatrices(); return viewProjectionMatrix; }

	const Mat4& getInverseViewProjectionMatrix(bool forceUpdate = false) const { if (forceUpdate) updateInverseMatrix(); return inverseViewProjectionMatrix; }
	Mat4 getInverseViewProjectionMatrixCopy(bool forceUpdate = false) const { if (forceUpdate) updateInverseMatrix(); return inverseViewProjectionMatrix; }

	uint64_t cameraHash() const {
		return globalTransformHash() ^ ((((uint64_t)(*(uint32_t*)&near) << 16 | (uint64_t)(*(uint32_t*)&far)) ^ ((uint64_t)width << 16 | (uint64_t)height)) ^ (uint64_t)(*(uint32_t*)&fovY) << 8);
	}

	const BRWL_STR& getName() { return name; }

private:
	void updateMatrices() const;
	void updateInverseMatrix() const;

	float fovY;
	float near;
	float far;
	int width;
	int height;


	mutable Mat4 viewMatrix; // = Inverse Model, daher Model = Inverse view
	mutable Mat4 inverseViewMatrix;

	mutable Mat4 projectionMatrix;
	mutable Mat4 inverseProjectionMatrix;

	mutable Mat4 viewProjectionMatrix;
	mutable Mat4 inverseViewProjectionMatrix;

	mutable bool projectionDirty;
	mutable bool inverseDirty;
	mutable uint64_t lastCameraHash;
};


BRWL_RENDERER_NS_END