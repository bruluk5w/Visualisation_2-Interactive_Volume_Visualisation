#pragma once // (c) 2020 Lukas Brunner

#include "Image.h"

BRWL_RENDERER_NS

struct TextureResource;

// Intended to be managed by PitCollections
class PitImage
{
public:
	PitImage(BRWL_STR name);
	virtual ~PitImage();

	Image cpuImage;
	ComPtr<ID3D12Fence> fence;
	uint64_t uploadFenceValue;
	HANDLE uploadEvent;
	std::unique_ptr<TextureResource> liveTexture;
	std::unique_ptr<TextureResource> stagedTexture;
};

BRWL_RENDERER_NS_END