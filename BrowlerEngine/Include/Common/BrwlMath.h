#pragma once // (c) 2020 Lukas Brunner

#include "BrwlMathFwd.h"

#ifdef BRWL_PLATFORM_WINDOWS
#include "DirectXMath.h"
namespace DirectX
{

	::BRWL::Vec2& operator-=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2& operator+=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2& operator*=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, float rhs);

	::BRWL::Vec2 operator*(float rhs, ::BRWL::Vec2 lhs);
}

#endif
