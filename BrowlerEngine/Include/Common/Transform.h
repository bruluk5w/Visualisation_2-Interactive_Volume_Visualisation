#pragma once // (c) 2020 Lukas Brunner
#include "Quaternion.h"
#include <vector>

BRWL_NS


class Hierarchy;
namespace RENDERER {
	class Camera;
}


class Transform
{
	friend class Hierarchy;
	friend class RENDERER::Camera;

	static const int initialChildCapacity = 4;
public:
	Transform(BRWL_STR name = BRWL_CHAR_LITERAL(""));
	// can't copy transforms since this could create invalid graphs that are not a tree anymore
	Transform(const Transform&) = delete;
	Transform& operator =(Transform const&) = delete;

	virtual ~Transform();
	// returns false if the parent could not be set due to a circular dependency
	bool setParent(Transform* parent);
	Transform* getParent() { return parent; }
	const Transform* getConstParent() const { return parent; }
	// begin() and end() allow range based for loop
	std::vector<Transform*>::iterator begin() { return children.begin(); }
	std::vector<Transform*>::iterator end() { return children.end(); }
	bool hasChildren() const { return !children.empty(); }
	size_t childCount() const { return children.size(); }

	Vec3 forward() const { return rot.forward(); }
	Vec3 up() const { return rot.up(); }
	Vec3 right() const { return rot.right(); }

	Vec3& position() { return pos; }
	Quaternion& rotation() { return rot; }
	Vec3& scaling() { return scale; }
	const Vec3& cPosition() const { return pos; }
	const Quaternion& cRotation() const { return rot; }
	const Vec3& cScaling() const { return scale; }
	Quaternion getGlobalOrientation() const;
	const Mat4& getLocalMatrix() const { return localMatrix; }
	// This matrix may only update with parent changes in the next frame
	const Mat4& getGlobalMatrix() const { return modelMatrix; }
	// This position may only update with parent changes in the next frame
	const Vec3 getGlobalPosition() const { return extractColumn3(modelMatrix, 3); }

	uint64_t localTransformHash() const;
	uint64_t globalTransformHash() const;

protected:

	BRWL_STR name;
	Transform* parent;
	std::vector<Transform*> children;
	Vec3 pos;
	Quaternion rot;
	Vec3 scale;

	Mat4 modelMatrix;
	Mat4 localMatrix;

	// this variable is set by Hierarchy and SceneObject (when initially loading) and used by Hierarchy to check if the transform has changed since the last frame
	uint64_t lastLocalTransformHash;
	uint64_t lastGlobalTransformHash;
};

BRWL_NS_END