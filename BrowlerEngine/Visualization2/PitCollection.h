#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/Texture.h"
#include "Renderer/BaseTextureManager.h"

BRWL_RENDERER_NS


// A collection of texture resources for preintegration tables
union PitCollection
{
	typedef TextureF32 PitImage;

	PitCollection(BaseTextureManager* mgr);
	~PitCollection();
	bool init(ID3D12Device* device);
	bool isResident() const;
	void destroy();

	// All our reintegration tables
	TextureHandle array[4];
	
	// aliases to distinguish in the rest of the code
	struct Aliases {
		TextureHandle refractionPit;
		TextureHandle particleColorPit;
		TextureHandle opacityPit;
		TextureHandle mediumColorPit;
	} tables;

	static_assert(sizeof(tables) == sizeof(array));
};


BRWL_RENDERER_NS_END
