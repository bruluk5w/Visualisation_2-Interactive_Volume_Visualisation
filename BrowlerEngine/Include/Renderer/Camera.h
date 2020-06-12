#pragma once // (c) 2020 Lukas Brunner

#include "Common/Transform.h"

BRWL_RENDERER_NS


class Camera : public Transform 
{
public:
	Camera(int viewportWidth, int viewportHeight, float fovY, float near, float far, const BRWL_CHAR* name = BRWL_CHAR_LITERAL("Camera"));

	void resize(int viewportWidth, int viewportHeight);

	void setHorizontalFov(float fovY) {
		this->fovY = fovY;
		inverseProjectionDirty = projectionDirty = true;
	}

	void setNearPlane(float zDepth) {
		near = zDepth;
		inverseProjectionDirty = projectionDirty = true;
	}

	void setFarPlane(float zDepth) {
		far = zDepth;
		inverseProjectionDirty = projectionDirty = true;
	}

	void setNearAndFarPlanes(float near, float far) {
		this->near = near;
		this->far = far;
		inverseProjectionDirty = projectionDirty = true;
	}

	float getNearPlane() const { return near; }

	float getFarPlane() const { return far; }

	const Mat4& getViewMatrix(bool forceUpdate = false) { updateMatrices(forceUpdate); return viewMatrix; }
	Mat4 getViewMatrixCopy(bool forceUpdate = false) { updateMatrices(forceUpdate); return viewMatrix; }

	const Mat4& getProjectionMatrix(bool forceUpdate = false) { updateMatrices(forceUpdate); return projectionMatrix; }
	Mat4 getProjectionMatrixCopy(bool forceUpdate = false) { updateMatrices(forceUpdate); return projectionMatrix; }

	const Mat4& getViewProjectionMatrix(bool forceUpdate = false) { updateMatrices(forceUpdate); return viewProjectionMatrix; }
	Mat4 getViewProjectionMatrixCopy(bool forceUpdate = false) { updateMatrices(forceUpdate); return viewProjectionMatrix; }

	const Mat4& getInverseViewProjectionMatrix(bool forceUpdate = false) { updateInverseMatrix(forceUpdate); return inverseViewProjectionMatrix; }
	Mat4 getInverseViewProjectionMatrixCopy(bool forceUpdate = false) { updateInverseMatrix(forceUpdate); return inverseViewProjectionMatrix; }

	uint64_t cameraHash() {
		lastGlobalTransformHash = globalTransformHash();
		return weakCameraHash();
	}
	
	uint64_t weakCameraHash() const {
		return lastGlobalTransformHash ^ localCameraHash();
	}

	const BRWL_STR& getName() { return name; }

private:
	uint64_t localCameraHash() const {
		return ((((uint64_t)(*(uint32_t*)&near) << 16 | (uint64_t)(*(uint32_t*)&far)) ^ ((uint64_t)width << 16 | (uint64_t)height)) ^ (uint64_t)(*(uint32_t*)&fovY) << 8);
	}
	bool updateMatrices(bool force);
	void updateInverseMatrix(bool force);

	float fovY;
	float near;
	float far;
	int width;
	int height;


	Mat4 viewMatrix; // = Inverse Model, daher Model = Inverse view
	Mat4 inverseViewMatrix;

	Mat4 projectionMatrix;
	Mat4 inverseProjectionMatrix;

	Mat4 viewProjectionMatrix;
	Mat4 inverseViewProjectionMatrix;

	bool projectionDirty;
	bool inverseProjectionDirty;
	uint64_t lastCameraHash;
};


BRWL_RENDERER_NS_END