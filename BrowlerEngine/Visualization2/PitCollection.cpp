#include "PitCollection.h"

#include "Renderer/TextureManager.h"

#include <algorithm>

BRWL_RENDERER_NS


PitCollection::PitCollection(BaseTextureManager* mgr) :
	tables {
		mgr->createTexture<PitImage>(BRWL_CHAR_LITERAL("Refraction PIT")),
		mgr->createTexture<PitImage>(BRWL_CHAR_LITERAL("Particle Color PIT")),
		mgr->createTexture<PitImage>(BRWL_CHAR_LITERAL("Opacity PIT")),
		mgr->createTexture<PitImage>(BRWL_CHAR_LITERAL("Medium Color PIT"))
	}
{ }


PitCollection::~PitCollection()
{
	destroy();
	tables.~Aliases();
}

bool PitCollection::init(ID3D12Device* device)
{
	//for (int i = 0; i < countof(array); ++i)
	//{
	//	TextureHandle& pit = array[i];
	//	if (!pit->create()) {
	//		return false;
	//	}
	//}

	return true;
}

bool PitCollection::isResident() const
{
	return std::all_of(array, array + countof(array), [](const TextureHandle& p) { return p.isResident(); });
}

void PitCollection::destroy()
{
	for (int i = 0; i < countof(array); ++i)
	{
		// destroy gpu resources
		array[i].destroy();
	}
}


BRWL_RENDERER_NS_END

