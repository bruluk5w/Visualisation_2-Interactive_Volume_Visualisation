#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

struct Quaternion;

struct BBox
{
	BBox() :
		min(0.f, 0.f, 0.f),
		max(0.f, 0.f, 0.f)
	{ }

	BBox(const Vec3& min, const Vec3& max) :
		min(min),
		max(max)
	{ }

	constexpr float dimX() const { return max.x - min.x; }
	constexpr float dimY() const { return max.y - min.y; }
	constexpr float dimZ() const { return max.z - min.z; }
	constexpr Vec3 dim() const { return { dimX(), dimY(), dimZ() }; }

	//! copies the maximum dimension to all axes
	void replicateMaxDim() {
		const float maxDim = Utils::max(Utils::max(dimX(), dimY()), dimZ());
		const float halfDiffX = (maxDim - dimX()) * 0.5f;
		const float halfDiffY = (maxDim - dimY()) * 0.5f;
		const float halfDiffZ = (maxDim - dimZ()) * 0.5f;
		min.x -= halfDiffX;
		min.y -= halfDiffY;
		min.z -= halfDiffZ;
		max.x += halfDiffX;
		max.y += halfDiffY;
		max.z += halfDiffZ;
	}

	/*!
	 * Returns the axis aligned bounding box relative to a coordinate system rotated by the given rotation matrix.
	 * The matrix is expcted to only rotate points.
	 */
	BBox getOBB (const Mat4& orientation) const;
	/*
	 * Returns the distance between the center of the bounding box and the nearest plane normal to a given direction which
	 * touches the bounding box. Like moving a plane normal to a given direction towards the bounding box as far as possible
	 * until it touches the box and measuring the smallest distance between the plane and the bbox center.
	 */

	float getClosestPlaneFromDirection(const Vec3& direction) const;

	Vec3 min, max;
};


BRWL_NS_END