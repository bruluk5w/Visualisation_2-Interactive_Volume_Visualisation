#include "BrwlMath.h"

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

	::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs) {
		XMStoreFloat2(&lhs, XMVectorAdd(XMVectorSet((float)lhs.x, (float)lhs.y, 0.f, 0.f), XMVectorSet((float)rhs.x, (float)rhs.y, 0.f, 0.f)));
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
}

#endif
