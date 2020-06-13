#include "BoundingBox.h"

#include "Quaternion.h"

BRWL_NS


BBox BBox::getOBB(const Quaternion& orientation)
{
	const Vec3 halfDim = dim() * 0.5f;
	const Vec3 right = orientation.right() * halfDim.x;
	const Vec3 top = orientation.up() * halfDim.y;
	const Vec3 front = orientation.forward() * halfDim.z;
	const Vec3 topRight = top + right;
	const Vec3 topLeft = top - right;
	const Vec3 bottomRight = right - top;
	const Vec3 bottomLeft = -right - top;

	Vec3 min{ 0 };
	Vec3 max{ 0 };
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


BRWL_NS_END
