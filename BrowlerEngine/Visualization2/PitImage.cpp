#include "PitImage.h"

#include "TextureResource.h"

BRWL_RENDERER_NS


PitImage::PitImage(BRWL_STR name) :
	cpuImage(name),
	uploadFenceValue(0),
	fence(nullptr)
{
	liveTexture = std::make_unique<TextureResource>();
	stagedTexture = std::make_unique<TextureResource>();

}

PitImage::~PitImage()
{ }


BRWL_RENDERER_NS_END
