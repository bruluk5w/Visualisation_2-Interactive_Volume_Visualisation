#include "BrwlMath.h"

#include "Quaternion.h"

BRWL_NS

const Vec3 zero = { 0.f, 0.f, 0.f };
const Vec3 one = { 1.f, 1.f, 1.f };
const Vec3 forward = { 0.f, 0.f, 1.f };
const Vec3 up = { 0.f, 1.f, 0.f };
const Vec3 right = { 1.f, 0.f, 0.f };

using namespace ::DirectX;
Vec2 normalize(Vec2 x) { XMStoreFloat2(&x, XMVector2Normalize(XMVectorSet(x.x, x.y, 0.f, 0.f))); return x; }
Vec3 normalize(Vec3 x) { XMStoreFloat3(&x, XMVector3Normalize(XMVectorSet(x.x, x.y, x.z, 0.f))); return x; }
Vec3 cross(Vec3 x, const Vec3& y) { XMStoreFloat3(&x, XMVector3Cross(XMVectorSet(x.x, x.y, x.z, 0.f), XMVectorSet(y.x, y.y, y.z, 0.f))); return x; }
Vec4 extractColumn4(const Mat4& x, size_t idx) { idx &= 0x3;  Vec4 y; y.x = ((float*)&x.r[0])[idx]; y.y = ((float*)&x.r[1])[idx]; y.z = ((float*)&x.r[2])[idx]; y.w = ((float*)&x.r[3])[idx]; return y; }
Vec3 extractColumn3(const Mat4& x, size_t idx) { idx &= 0x3;  Vec3 y; y.x = ((float*)&x.r[0])[idx]; y.y = ((float*)&x.r[1])[idx]; y.z = ((float*)&x.r[2])[idx]; return y; }
Vec3 extractPosition(const Mat4& x) { return Vec3(((float*)&x.r)[12], ((float*)&x.r)[13], ((float*)&x.r)[14]); }
Vec3 min(const Vec3& a, const Vec3& b) { Vec3 res; XMStoreFloat3(&res, XMVectorMin(XMVectorSet(a.x, a.y, a.z, 0.f), XMVectorSet(b.x, b.y, b.z, 0.f))); return res; }
Vec3& storeMin(Vec3& a, const Vec3& b) { XMStoreFloat3(&a, XMVectorMin(XMVectorSet(a.x, a.y, a.z, 0.f), XMVectorSet(b.x, b.y, b.z, 0.f))); return a; }
Vec3 max(const Vec3& a, const Vec3& b) { Vec3 res; XMStoreFloat3(&res, XMVectorMax(XMVectorSet(a.x, a.y, a.z, 0.f), XMVectorSet(b.x, b.y, b.z, 0.f))); return res; }
Vec3& storeMax(Vec3& a, const Vec3& b) { XMStoreFloat3(&a, XMVectorMax(XMVectorSet(a.x, a.y, a.z, 0.f), XMVectorSet(b.x, b.y, b.z, 0.f))); return a; }
Mat4 makePerspective(float fovY, float aspect, float near, float far) { return XMMatrixPerspectiveFovLH(fovY, aspect, near, far); }
Mat4 makeOrthographic(float width, float height, float near, float far) { return XMMatrixOrthographicLH(width, height, near, far); }
Mat4 makeAffineTransform(const Vec3& pos, const Vec3& rot, const Vec3& scale) { return XMMatrixAffineTransformation(XMVectorSet(scale.x, scale.y, scale.z, 0), XMVectorSet(0, 0, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMVectorSet(rot.x, rot.y, rot.z, 0)), XMVectorSet(pos.x, pos.y, pos.z, 0)); }
Mat4 makeAffineTransform(const Vec3& pos, const Quaternion& rot, const Vec3& scale)
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
		pos.x * scale.x, pos.y * scale.y, pos.z * scale.z, 1.0f
	);
}


Mat4 inverse(const Mat4& m) { return XMMatrixInverse(nullptr, m); }
Mat4 identity() { return XMMatrixIdentity(); }


BRWL_NS_END

#ifdef BRWL_PLATFORM_WINDOWS
#include "DirectXMath.h"
namespace DirectX
{

	::BRWL::Vec2& operator-=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorNegate(XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f))));
		return lhs;
	}

	::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorNegate(XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f))));
		return lhs;
	}

	::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, float rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorNegate(XMVectorSet(rhs, rhs, 0.f, 0.f))));
		return lhs;
	}

	::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, float rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet(rhs, rhs, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec2& operator+=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec2& operator*=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, float rhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet(rhs, rhs, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec2 operator*(float rhs, ::BRWL::Vec2 lhs) {
		XMStoreFloat2(&lhs, XMVectorMultiply(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet(rhs, rhs, 0.f, 0.f)));
		return lhs;
	}

	::BRWL::Vec3& operator-=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorNegate(XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f))));
		return lhs;
	}

	::BRWL::Vec3 operator-(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorNegate(XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f))));
		return lhs;
	}

	::BRWL::Vec3 operator-(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorNegate(XMVectorSet(rhs, rhs, rhs, 0.f))));
		return lhs;
	}

	::BRWL::Vec3 operator-(const ::BRWL::Vec3& v)
	{
		::BRWL::Vec3 res;
		XMStoreFloat3(&res, XMVectorNegate(XMVectorSet(v.x, v.y, v.z, 0.f)));
		return res;
	}

	::BRWL::Vec3 operator+(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	::BRWL::Vec3 operator+(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs, rhs, rhs, 0.f)));
		return lhs;
	}

	::BRWL::Vec3& operator+=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorAdd(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	float operator*(const ::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		float res;
		XMStoreFloat(&res, XMVector3Dot(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return res;
	}

	::BRWL::Vec3& operator*=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	::BRWL::Vec3& operator*=(::BRWL::Vec3& lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet((float)lhs.x, (float)lhs.y, (float)lhs.z, 0.f), XMVectorReplicate(rhs)));
		return lhs;
	}

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorReplicate(rhs)));
		return lhs;
	}

	::BRWL::Vec3 operator/(::BRWL::Vec3 lhs, float rhs) {
		const float recip = 1.f / rhs;
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorReplicate(recip)));
		return lhs;
	}


	::BRWL::Vec3 operator*(float rhs, ::BRWL::Vec3 lhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs, rhs, rhs, 0.f)));
		return lhs;
	}

	::BRWL::Vec4 operator*(::BRWL::Vec4 lhs, ::BRWL::Mat4 rhs)
	{
		XMVECTOR v = XMVector4Transform(XMVectorDivide(XMVectorSet(lhs.x, lhs.y, lhs.z, lhs.w), XMVectorReplicate(lhs.w)), rhs);

		XMStoreFloat4(&lhs, XMVectorDivide(v, XMVectorSplatW(v)));
		return lhs;
	}

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, ::BRWL::Mat4 rhs)
	{
		XMStoreFloat3(&lhs, XMVector3TransformCoord(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), rhs));
		return lhs;
	}
}

#endif
