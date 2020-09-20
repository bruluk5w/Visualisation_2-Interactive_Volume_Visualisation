#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/Texture.h"

BRWL_RENDERER_NS

// A collection of texture resources for preintegration tables
union PitCollection
{
	typedef RENDERER::TextureF32 PitImage;
	PitCollection(RENDERER::TextureManager* mgr);
	~PitCollection();
	bool init(ID3D12Device* device);
	bool isResident();
	void destroy();

	// All our reintegration tables
	Texture<float> array[4];
	
	// aliases to distinguish in the rest of the code
	struct Aliases {
		PitImage refractionPit;
		PitImage particleColorPit;
		PitImage opacityPit;
		PitImage mediumColorPit;
	} tables;

	static_assert(sizeof(tables) == sizeof(array));
};



BRWL_RENDERER_NS_END