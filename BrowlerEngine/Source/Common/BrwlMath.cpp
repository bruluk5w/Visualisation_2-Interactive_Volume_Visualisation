#include "BrwlMath.h"

#include "Quaternion.h"

BRWL_NS


using namespace DirectX;
Vec2 normalize(Vec2 x) { XMStoreFloat2(&x, XMVector2Normalize(XMVectorSet(x.x, x.y, 0.f, 0.f))); return x; }
Vec3 normalize(Vec3 x) { XMStoreFloat3(&x, XMVector3Normalize(XMVectorSet(x.x, x.y, x.z, 0.f))); return x; }
Vec3 cross(Vec3 x, const Vec3& y) { XMStoreFloat3(&x, XMVector3Cross(XMVectorSet(x.x, x.y, x.z, 0.f), XMVectorSet(y.x, y.y, y.z, 0.f))); return x; }
Vec4 extractColumn4(const Mat4& x, size_t idx) { idx &= 0x3;  Vec4 y; y.x = ((float*)&x.r[0])[idx]; y.y = ((float*)&x.r[1])[idx]; y.z = ((float*)&x.r[2])[idx]; y.w = ((float*)&x.r[3])[idx]; return y; }
Vec3 extractColumn3(const Mat4& x, size_t idx) { idx &= 0x3;  Vec3 y; y.x = ((float*)&x.r[0])[idx]; y.y = ((float*)&x.r[1])[idx]; y.z = ((float*)&x.r[2])[idx]; return y; }
Mat4 makePerspective(float fovY, float aspect, float near, float far) { return XMMatrixPerspectiveFovLH(fovY, aspect, near, far); }

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

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	::BRWL::Vec3& operator*=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs.x, rhs.y, rhs.z, 0.f)));
		return lhs;
	}

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, float rhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs, rhs, rhs, 0.f)));
		return lhs;
	}

	::BRWL::Vec3 operator*(float rhs, ::BRWL::Vec3 lhs) {
		XMStoreFloat3(&lhs, XMVectorMultiply(XMVectorSet(lhs.x, lhs.y, lhs.z, 0.f), XMVectorSet(rhs, rhs, rhs, 0.f)));
		return lhs;
	}

}

#endif
