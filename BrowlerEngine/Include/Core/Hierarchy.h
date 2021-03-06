#pragma once // (c) 2020 Lukas Brunner

#include "Common/Transform.h"

BRWL_NS


class Hierarchy
{
public:
	void update();

	bool addToRoot(Transform* child)
	{
		BRWL_EXCEPTION(child->getConstParent() == nullptr, BRWL_CHAR_LITERAL("Only Transforms with no parent can be added to the root of the Hierarchy"));
		return child->setParent(&root);
	}

private:
	void update(Transform* parent, bool parentChanged, uint64_t parentGlobalHash);
	Transform root;
};


BRWL_NS_END