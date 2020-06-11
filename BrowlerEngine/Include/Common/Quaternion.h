#pragma once // (c) 2020 Lukas Brunner

BRWL_NS


#pragma once

struct Quaternion
{
	static const Quaternion identity;

	Quaternion();

	Quaternion(float x, float y, float z, float w);

	// axis is expected to be normalized
	Quaternion(const Vec3& axis, float angle);

	Quaternion(const Quaternion& other);

	explicit operator Vec4() const;

	static void toAxisAngle(const Quaternion& q, Vec3& outAxis, float& outAngle);

	static Vec3 toEuler(const Quaternion& q);

	Vec3 toEuler();

	static Quaternion makeFromEuler(const Vec3& euler);

	Quaternion& fromEuler(const Vec3& euler);

	Quaternion& fromEuler(float eulerX, float eulerY, float eulerZ);

	static void fromAxisAngle(const Vec3& axis, float angle, Quaternion& out);

	static void fromAxisAngle(float axisX, float axisY, float axisZ, float angle, Quaternion& out);

	static Quaternion lookAt(Vec3 from, Vec3 to);

	float magnitude();

	float angle();

	Quaternion& normalize();

	Quaternion operator*(const Quaternion& other) const;

	Quaternion& operator*=(const Quaternion& other);

	Quaternion operator*(float f) const;

	Quaternion& operator*=(float f);

	// rotates a vector by the rotation described by this quaternion
	Vec3 operator*(const Vec3& rhs) const;

	Quaternion inverse() const;

	Quaternion& invert();

	void set(float x, float y, float z, float w);

	void set(Quaternion other);

	bool operator==(const Quaternion& other) const;

	bool operator!=(const Quaternion& other) const;

	Vec3 forward() const;
	Vec3 up() const;
	Vec3 right() const;

	float x, y, z, w;
};


BRWL_NS_END