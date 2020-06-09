#include "Hierarchy.h"

BRWL_NS


void Hierarchy::update()
{
	uint64_t localTransformHash = root.localTransformHash();
	bool parentChanged = false;
	if (localTransformHash != root.localTransformHashLastFrame)
	{
		root.localTransformHashLastFrame = localTransformHash;
		parentChanged = true;
		root.localMatrix = makeAffineTransform(root.pos, root.rot, root.scale);
		root.modelMatrix = root.localMatrix;
	}

	update(&root, parentChanged);
}

void Hierarchy::update(Transform* parent, bool parentChanged)
{
	for (Transform* child : *parent) {
		uint64_t localTransformHash = child->localTransformHash();
		if (localTransformHash != child->localTransformHashLastFrame)
		{
			child->localTransformHashLastFrame = localTransformHash;
			parentChanged = true;
			child->localMatrix = makeAffineTransform(child->pos, child->rot, child->scale);
		}

		if (parentChanged)
		{
			child->modelMatrix = parent->modelMatrix * child->localMatrix;
		}

		update(child, parentChanged);
	}
}


BRWL_NS_END