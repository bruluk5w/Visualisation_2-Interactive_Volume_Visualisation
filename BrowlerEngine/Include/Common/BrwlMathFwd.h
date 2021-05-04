#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS
namespace DirectX
{
	struct XMFLOAT2;
	struct XMFLOAT2A;
	struct XMFLOAT3;
	struct XMFLOAT3A;
	struct XMFLOAT4;
	struct XMFLOAT4A;
	struct XMMATRIX;
	struct XMFLOAT3X3;
}
#endif

BRWL_NS

#ifdef BRWL_PLATFORM_WINDOWS


typedef DirectX::XMFLOAT2 Vec2;
typedef DirectX::XMFLOAT2A Vec2a;
typedef DirectX::XMFLOAT3  Vec3;
typedef DirectX::XMFLOAT3A Vec3a;
typedef DirectX::XMFLOAT4  Vec4;
typedef DirectX::XMFLOAT4A Vec4a;
typedef DirectX::XMMATRIX Mat4;
typedef DirectX::XMFLOAT3X3 Mat3;

#endif

BRWL_NS_END
