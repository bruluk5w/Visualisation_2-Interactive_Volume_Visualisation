#include "Transform.h"

#include <algorithm>

BRWL_NS



Transform::Transform(BRWL_STR name) :
	name(std::move(name)),
	parent(nullptr),
	children(),
	pos(),
	rot(),
	scale(1, 1, 1),
	modelMatrix(),
	localMatrix(),
	lastLocalTransformHash(0)
{
	children.reserve(initialChildCapacity);
}

Transform::~Transform()
{
	//EXCEPTION(!childCount(), "A transform is destroyed while children are still referencing it! Transforms should be destroyed from leaf to root.");
	BRWL_VERIFY(setParent(nullptr), BRWL_CHAR_LITERAL("Failed to remove reference to/from parent"));
}

bool Transform::setParent(Transform* newParent)
{
	if (newParent == parent)
	{
		return true;
	}

	Transform* current = this;
	do {
		current = current->getParent();
		if (!BRWL_VERIFY(current != this, BRWL_CHAR_LITERAL("Cannot create a circular transform dependency.")))
		{
			return false;
		}
	} while (current != nullptr);

	if (parent != nullptr)
	{	// remove self from previous parent's child list
		parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
	}

	this->parent = newParent;
	if (parent != nullptr)
	{
		parent->children.emplace_back(this);
	}

	return true;
}

uint64_t Transform::localTransformHash() const
{
	return (((uint64_t)(*(uint32_t*)&scale.x) << 16 | (uint64_t)(*(uint32_t*)&scale.y)) ^ (uint64_t)(*(uint32_t*)&scale.z) << 8) ^
		(((uint64_t)(*(uint32_t*)&pos.x) << 16 | (uint64_t)(*(uint32_t*)&pos.y)) ^ (uint64_t)(*(uint32_t*)&pos.z) << 8) ^
		((uint64_t)(*(uint32_t*)&rot.x) << 16 | (uint64_t)(*(uint32_t*)&rot.y) ^ ((uint64_t)(*(uint32_t*)&rot.z) << 16 | (uint64_t)(*(uint32_t*)&rot.w)));
}

uint64_t Transform::globalTransformHash() const
{
	uint64_t hash = localTransformHash();
	if (parent != nullptr)
	{
		hash ^= parent->globalTransformHash();
	}

	return hash;
}

Quaternion Transform::getGlobalOrientation() const
{
	Quaternion q = rot;
	const Transform* t = this;
	while (t->getConstParent()) {
		t = t->getConstParent();
		q *= t->rot;
	}

	return q;
}


BRWL_NS_END