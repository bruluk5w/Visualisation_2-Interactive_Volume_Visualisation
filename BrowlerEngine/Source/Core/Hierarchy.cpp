#include "Hierarchy.h"

BRWL_NS


void Hierarchy::update()
{
	uint64_t localTransformHash = root.localTransformHash();
	bool parentChanged = false;
	if (localTransformHash != root.lastLocalTransformHash)
	{
		root.lastLocalTransformHash = localTransformHash;
		root.lastGlobalTransformHash = localTransformHash;
		parentChanged = true;
		root.localMatrix = makeAffineTransform(root.pos, root.rot, root.scale);
		root.modelMatrix = root.localMatrix;
	}

	update(&root, parentChanged, localTransformHash);
}

void Hierarchy::update(Transform* parent, bool parentChanged, uint64_t parentGlobalHash)
{
	for (Transform* child : *parent) {
		uint64_t localTransformHash = child->localTransformHash();
		if (localTransformHash != child->lastLocalTransformHash)
		{
			child->lastLocalTransformHash = localTransformHash;
			child->lastGlobalTransformHash = localTransformHash ^ parentGlobalHash;
			parentChanged = true;
			child->localMatrix = makeAffineTransform(child->pos, child->rot, child->scale);
		}

		if (parentChanged)
		{
			child->modelMatrix = parent->modelMatrix * child->localMatrix;
		}

		update(child, parentChanged, child->lastGlobalTransformHash);
	}
}


BRWL_NS_END