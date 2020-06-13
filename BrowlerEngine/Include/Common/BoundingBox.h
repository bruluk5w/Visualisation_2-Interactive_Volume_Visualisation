#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

struct Quaternion;

struct BBox
{
	BBox() :
		min(-0.5f, -0.5f, -0.5f),
		max(-0.5f, -0.5f, -0.5f)
	{ }

	BBox(const Vec3& min, const Vec3& max) :
		min(min),
		max(max)
	{ }

	constexpr float dimX() const { return max.x - min.x; }
	constexpr float dimY() const { return max.y - min.y; }
	constexpr float dimZ() const { return max.z - min.z; }
	constexpr Vec3 dim() const { return { dimX(), dimY(), dimZ() }; }
	BBox getOBB(const Quaternion& orientation);

	Vec3 min, max;
};


BRWL_NS_END