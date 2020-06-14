#include "BoundingBox.h"

#include "Quaternion.h"

BRWL_NS


BBox BBox::getOBB (const Quaternion& orientation) const
{
	const Vec3 halfDim = dim() * 0.5f;
	const Vec3 right = orientation.right() * halfDim.x;
	const Vec3 top = orientation.up() * halfDim.y;
	const Vec3 front = orientation.forward() * halfDim.z;
	const Vec3 topRight = top + right;
	const Vec3 topLeft = top - right;
	const Vec3 bottomRight = right - top;
	const Vec3 bottomLeft = -right - top;

	Vec3 min{ 0, 0, 0 };
	Vec3 max{ 0, 0, 0 };
	Vec3 corner = topRight + front;
	storeMin(min, corner); storeMax(max, corner);
	corner = topLeft + front;
	storeMin(min, corner); storeMax(max, corner);
	corner = bottomRight + front;
	storeMin(min, corner); storeMax(max, corner);
	corner = bottomLeft + front;
	storeMin(min, corner); storeMax(max, corner);
	corner = topRight - front;
	storeMin(min, corner); storeMax(max, corner);
	corner = topLeft - front;
	storeMin(min, corner); storeMax(max, corner);
	corner = bottomRight - front;
	storeMin(min, corner); storeMax(max, corner);
	corner = bottomLeft - front;
	storeMin(min, corner); storeMax(max, corner);

    return BBox(min, max);
}

float BBox::getClosestPlaneFromDirection(const Vec3 & direction) const
{
	const Vec3 dir = normalized(direction);
	float maxDist = std::numeric_limits<float>::lowest();
	{
		const Vec3 corner(min.x, min.y, min.z);
		const float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(min.x, min.y, max.z);
		const float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(min.x, max.y, min.z);
		const float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(min.x, max.y, max.z);
		const float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(max.x, min.y, min.z);
		const float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(max.x, min.y, max.z);
		float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(max.x, max.y, min.z);
		float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}
	{
		const Vec3 corner(max.x, max.y, max.z);
		const float dist = corner * dir;
		if (dist > maxDist) maxDist = dist;
	}

	return maxDist;
}

BRWL_NS_END
