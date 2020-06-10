#pragma once // (c) 2020 Lukas Brunner

#include "BrwlMathFwd.h"

#ifdef BRWL_PLATFORM_WINDOWS

#include "DirectXMath.h"

BRWL_NS


struct Quaternion;

extern const Vec3 forward;
extern const Vec3 up;
extern const Vec3 right;

Vec2 normalize(Vec2 x);
Vec3 normalize(Vec3 x);
Vec3 cross(Vec3 x, const Vec3& y);
Vec4 extractColumn4(const Mat4& x, size_t idx);
Vec3 extractColumn3(const Mat4& x, size_t idx);
Mat4 makePerspective(float fovY, float aspect, float near, float far);
Mat4 makeOrthographic(float width, float height, float near, float far);
Mat4 makeAffineTransform(const Vec3& pos, const Quaternion& rot, const Vec3& scale);
Mat4 makeAffineTransform(const Vec3& pos, const Vec3& rot, const Vec3& scale);
Mat4 inverse(const Mat4& m);
Mat4 identity();


BRWL_NS_END

namespace DirectX
{
	::BRWL::Vec2& operator-=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator-(::BRWL::Vec2 lhs, float rhs);

	::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator+(::BRWL::Vec2 lhs, float rhs);

	::BRWL::Vec2& operator+=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2& operator*=(::BRWL::Vec2& lhs, const ::BRWL::Vec2& rhs);

	::BRWL::Vec2 operator*(::BRWL::Vec2 lhs, float rhs);

	::BRWL::Vec2 operator*(float rhs, ::BRWL::Vec2 lhs);

	::BRWL::Vec3& operator-=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs);

	::BRWL::Vec3 operator-(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs);

	::BRWL::Vec3 operator-(::BRWL::Vec3 lhs, float rhs);

	::BRWL::Vec3 operator+(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs);

	::BRWL::Vec3 operator+(::BRWL::Vec3 lhs, float rhs);

	::BRWL::Vec3& operator+=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs);

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, const ::BRWL::Vec3& rhs);

	::BRWL::Vec3& operator*=(::BRWL::Vec3& lhs, const ::BRWL::Vec3& rhs);

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, float rhs);

	::BRWL::Vec3 operator*(float rhs, ::BRWL::Vec3 lhs);

	::BRWL::Vec4 operator*(::BRWL::Vec4 lhs, ::BRWL::Mat4 rhs);

	::BRWL::Vec3 operator*(::BRWL::Vec3 lhs, ::BRWL::Mat4 rhs);

}
#endif
