#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/BaseTextureHandle.h"
#include "Renderer/Texture.h"

BRWL_RENDERER_NS


// A collection of textures for preintegration tables
union PitCollection
{
	typedef TextureF32 PitImage;
	typedef TextureVEC4F32 ColorPitImage;

	PitCollection();
	~PitCollection();
	void create(BaseTextureManager* mgr);
	bool isResident() const;
	void destroy();

	// All our reintegration tables
	BaseTextureHandle array[4];
	
	// aliases to distinguish in the rest of the code
	struct Aliases {
		BaseTextureHandle refractionPit;
		BaseTextureHandle particleColorPit;
		BaseTextureHandle opacityPit;
		BaseTextureHandle mediumColorPit;
	} tables;

	static_assert(sizeof(tables) == sizeof(array));
};


BRWL_RENDERER_NS_END
