#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS
namespace DirectX
{
	struct XMFLOAT3;
	struct XMFLOAT3A;
	struct XMFLOAT4;
	struct XMFLOAT4A;
}
#endif

BRWL_NS

#ifdef BRWL_PLATFORM_WINDOWS


typedef DirectX::XMFLOAT3  Vec3;
typedef DirectX::XMFLOAT3A Vec3a;
typedef DirectX::XMFLOAT4  Vec4;
typedef DirectX::XMFLOAT4A Vec4a;

#endif

BRWL_NS_END
