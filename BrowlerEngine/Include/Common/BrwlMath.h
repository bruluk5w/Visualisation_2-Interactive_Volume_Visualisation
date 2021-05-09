#pragma once // (c) 2020 Lukas Brunner

#include "BrwlMathFwd.h"

#include "Quaternion.h"

#ifdef BRWL_PLATFORM_WINDOWS
// TODO: replace with SimpleMath types 
#include "DirectXMath.h"

BRWL_NS


struct Quaternion;

extern const Vec3 VEC3_ZERO;
extern const Vec3 VEC3_ONE;
extern const Vec3 VEC3_FWD;
extern const Vec3 VEC3_UP;
extern const Vec3 VEC3_RIGHT;

extern const Vec4 VEC4_ZERO;
extern const Vec4 VEC4_ONE;
extern const Vec4 VEC4_FWD;
extern const Vec4 VEC4_UP;
extern const Vec4 VEC4_RIGHT;

inline Vec3 toVec3(const Vec4& x) { return Vec3(x.x, x.y, x.z); }

using namespace ::DirectX;
inline Vec2& normalize(Vec2& x) { XMStoreFloat2(&x, XMVector2Normalize(XMLoadFloat2(&x))); return x; }
inline Vec2 normalized(Vec2 x) { XMStoreFloat2(&x, XMVector2Normalize(XMLoadFloat2(&x))); return x; }
inline Vec3& normalize(Vec3& x) { XMStoreFloat3(&x, XMVector3Normalize(XMLoadFloat3(&x))); return x; }
inline Vec3 normalized(Vec3 x) { XMStoreFloat3(&x, XMVector3Normalize(XMLoadFloat3(&x))); return x; }
inline float length(const Vec3& x) { float y;  XMStoreFloat(&y, XMVector3Length(XMLoadFloat3(&x))); return y; }
inline float lengthSq(const Vec3& x) { float y;  XMStoreFloat(&y, XMVector3LengthSq(XMLoadFloat3(&x))); return y; }
inline Vec3 cross(Vec3 x, const Vec3& y) { XMStoreFloat3(&x, XMVector3Cross(XMLoadFloat3(&x), XMLoadFloat3(&y))); return x; }
inline Vec4 extractColumn4(const Mat4& x, size_t idx) { idx &= 0x3;  Vec4 y; y.x = ((float*)&x.r[0])[idx]; y.y = ((float*)&x.r[1])[idx]; y.z = ((float*)&x.r[2])[idx]; y.w = ((float*)&x.r[3])[idx]; return y; }
inline Vec3 extractColumn3(const Mat4& x, size_t idx) { idx &= 0x3;  Vec3 y; y.x = ((float*)&x.r[0])[idx]; y.y = ((float*)&x.r[1])[idx]; y.z = ((float*)&x.r[2])[idx]; return y; }
inline Vec3 extractPosition(const Mat4& x) { return Vec3(((float*)&x.r)[12], ((float*)&x.r)[13], ((float*)&x.r)[14]); }
inline Vec3 min(const Vec3& a, const Vec3& b) { Vec3 res; XMStoreFloat3(&res, XMVectorMin(XMLoadFloat3(&a), XMLoadFloat3(&b))); return res; }
inline Vec3& storeMin(Vec3& a, const Vec3& b) { XMStoreFloat3(&a, XMVectorMin(XMLoadFloat3(&a), XMLoadFloat3(&b))); return a; }
inline Vec3 max(const Vec3& a, const Vec3& b) { Vec3 res; XMStoreFloat3(&res, XMVectorMax(XMLoadFloat3(&a), XMLoadFloat3(&b))); return res; }
inline Vec3& storeMax(Vec3& a, const Vec3& b) { XMStoreFloat3(&a, XMVectorMax(XMLoadFloat3(&a), XMLoadFloat3(&b))); return a; }
inline Mat4 makePerspective(float fovY, float aspect, float near, float far) { return XMMatrixPerspectiveFovLH(fovY, aspect, near, far); }
inline Mat4 makeOrthographic(float width, float height, float near, float far) { return XMMatrixOrthographicLH(width, height, near, far); }
inline Mat4 makeAffineTransform(const Vec3& pos, const Vec3& rot, const Vec3& scale) { return XMMatrixAffineTransformation(XMLoadFloat3(&scale), XMVectorSplatConstantInt(0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rot)), XMLoadFloat3(&pos)); }
inline Mat4 makeAffineTransform(const Vec3& pos = VEC3_ZERO, const Quaternion& rot = Quaternion::identity, const Vec3& scale = Vec3(1, 1, 1))
{
	const float xx = rot.x * rot.x;
	const float xy = rot.x * rot.y;
	const float xz = rot.x * rot.z;
	const float xw = rot.x * rot.w;
	const float yy = rot.y * rot.y;
	const float yz = rot.y * rot.z;
	const float yw = rot.y * rot.w;
	const float zz = rot.z * rot.z;
	const float zw = rot.z * rot.w;
	return Mat4(
		1.0f - 2.0f * (yy + zz), 2.0f * (xy + zw), 2.0f * (xz - yw), 0.0f,
		2.0f * (xy - zw), 1.0f - 2.0f * (xx + zz), 2.0f * (yz + xw), 0.0f,
		2.0f * (xz + yw), 2.0f * (yz - xw), 1.0f - 2.0f * (xx + yy), 0.0f,
		pos.x, pos.y, pos.z, 1.0f
	) * XMMatrixScalingFromVector(XMVectorSet(scale.x, scale.y, scale.z, 1.0));
}
inline Mat4 makeLookAtTransform(const Vec3& from, const Vec3& to, const Vec3& up = VEC3_UP) { return XMMatrixLookAtLH(XMLoadFloat3(&from), XMLoadFloat3(&to), XMLoadFloat3(&up)); }
inline Mat4 makeRotationMatrix(const Vec4& q) { return XMMatrixRotationQuaternion(XMLoadFloat4(&q)); }
inline Mat4 inverse(const Mat4& m) { return XMMatrixInverse(nullptr, m); }
inline Mat3 inverse(const Mat3& m) { Mat3 n; XMStoreFloat3x3(&n, XMMatrixInverse(nullptr, XMLoadFloat3x3(&m))); return n; }
inline Mat3 transpose(const Mat3& m) { Mat3 n; XMStoreFloat3x3(&n, XMMatrixTranspose(XMLoadFloat3x3(&m))); return n; }

BRWL_NS_END


namespace DirectX
{
	inline bool operator==(const ::BRWL::Vec2& a, const ::BRWL::Vec2& b)
	{
		return XMVector3EqualR(XMLoadFloat2(&a), XMLoadFloat2(&b));
	}

	inline bool operator!=(const ::BRWL::Vec2& a, const ::BRWL::Vec2& b)
	{
		return !(a == b);
	}

	inline ::BRWL::Vec2& operator-=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorNegate(XMVectorSet(rhs.x, rhs.y, 0.f, 0.f))));
		return lhs;
	}

	inline ::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorNegate(XMVectorSet(rhs.x, rhs.y, 0.f, 0.f))));
		return lhs;
	}

	inline ::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, float rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorNegate(XMVectorSet(rhs, rhs, 0.f, 0.f))));
		return lhs;
	}

	inline ::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs.x, rhs.y, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, float rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs, rhs, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec2& operator+=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs.x, rhs.y, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs.x, rhs.y, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec2& operator*=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs.x, rhs.y, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, float rhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs, rhs, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec2 operator*(float lhs, ::BRWL::Vec2 rhs) {
		XMStoreFloat2(&rhs, XMVectorMultiply(XMVectorSet(rhs.x, rhs.y, 0.f, 0.f), XMVectorSet(lhs, lhs, 0.f, 0.f)));
		return rhs;
	}

	inline ::BRWL::Vec2 operator/(::BRWL::Vec2 lhs, ::BRWL::Vec2 rhs) {
		XMStoreFloat2(&lhs, XMVectorDivide(XMVectorSet(lhs.x, lhs.y, 0.f, 0.f), XMVectorSet(rhs.x, rhs.y, 0.f, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec3& operator-=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorNegate(XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f))));
		return lhs;
	}

	inline ::BRWL::Vec3 operator-(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorNegate(XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f))));
		return lhs;
	}

	inline ::BRWL::Vec3 operator-(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorNegate(XMVectorSet(rhs, rhs, rhs, 0.f))));
		return lhs;
	}

	inline ::BRWL::Vec3 operator-(const ::BRWL::Vec3& v)
	{
		::BRWL::Vec3 res;
		XMStoreFloat3(&res, XMVectorNegate(XMVectorSet(v.x, v.y, v.z, 0.f)));
		return res;
	}

	inline ::BRWL::Vec3 operator+(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec3 operator+(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs, rhs, rhs, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec3& operator+=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	inline float operator*(const ::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		float res;
		XMStoreFloat(&res, XMVector3Dot(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return res;
	}

	inline ::BRWL::Vec3& operator*=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec3& operator*=(::BRWL::Vec3& lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet((float)lhs.x, (float)lhs.y, (float)lhs.z, 0.f), XMVectorReplicate(rhs)));
		return lhs;
	}

	inline ::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorReplicate(rhs)));
		return lhs;
	}

	inline ::BRWL::Vec3 operator/(::BRWL::Vec3 lhs, float rhs) {
		const float recip = 1.f / rhs;
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorReplicate(recip)));
		return lhs;
	}


	inline ::BRWL::Vec3 operator*(float rhs, ::BRWL::Vec3 lhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs, rhs, rhs, 0.f)));
		return lhs;
	}

	inline ::BRWL::Vec4 operator*(::BRWL::Vec4 lhs, ::BRWL::Mat4 rhs)
	{
		if (lhs.w != 0)
		{
			XMVECTOR tmp = XMVector4Transform(XMVectorDivide(XMVectorSet(lhs.x, lhs.y, lhs.z, lhs.w), XMVectorReplicate(lhs.w)), rhs);
			XMStoreFloat4(&lhs, XMVectorDivide(tmp, XMVectorSplatW(tmp)));
		}
		else
		{
			XMStoreFloat4(&lhs, XMVector4Transform(XMVectorSet(lhs.x, lhs.y, lhs.z, lhs.w), rhs));
		}

		return lhs;
	}

	inline ::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, ::BRWL::Mat4 rhs)
	{
		XMStoreFloat3(&lhs, XMVector3TransformCoord(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), rhs));
		return lhs;
	}

	inline ::BRWL::Vec4 operator-(::BRWL::Vec4 v)
	{
		XMStoreFloat4(&v, XMVectorNegate(XMVectorSet(v.x, v.y, v.z, v.w)));
		return v;
	}

	inline bool operator==(const ::BRWL::Vec3& a, const ::BRWL::Vec3& b)
	{
		return XMVector3EqualR(XMVectorSet(a.x, a.y, a.z, 0.f), XMVectorSet(b.x, b.y, b.z, 0.f));
	}

	inline bool operator!=(const ::BRWL::Vec3& a, const ::BRWL::Vec3& b)
	{
		return !(a == b);
	}

	inline bool operator==(const ::BRWL::Vec4& a, const ::BRWL::Vec4& b)
	{
		return XMVector3EqualR(XMVectorSet(a.x, a.y, a.z, a.w), XMVectorSet(b.x, b.y, b.z, b.w));
	}

	inline bool operator!=(const ::BRWL::Vec4& a, const ::BRWL::Vec4& b)
	{
		return !(a == b);
	}
}

#endif
