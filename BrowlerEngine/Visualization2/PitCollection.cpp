#include "PitCollection.h"

#include "Renderer/TextureManager.h"

#include <algorithm>

BRWL_RENDERER_NS


PitCollection::PitCollection(RENDERER::TextureManager* mgr) :
	tables {
		{ BRWL_CHAR_LITERAL("Refraction PIT"), mgr },
		{ BRWL_CHAR_LITERAL("Particle Color PIT"), mgr },
		{ BRWL_CHAR_LITERAL("Opacity PIT"), mgr },
		{ BRWL_CHAR_LITERAL("Medium Color PIT"), mgr }
	}
{ }


PitCollection::~PitCollection()
{
	tables.~Aliases();
}

bool PitCollection::init(ID3D12Device* device)
{
	for (int i = 0; i < countof(array); ++i)
	{
		PitImage& pit = array[i];
		if (!pit.initGpu()) {
			return false;
		}
	}

	return true;
}

bool PitCollection::isResident()
{
	return std::all_of(array, array + countof(array), [](const PitImage& p) { return p.isResident(); });
}

void PitCollection::destroy()
{
	for (int i = 0; i < countof(array); ++i)
	{
		PitImage& pit = array[i];
		pit.destroyGpu();
	}
}


BRWL_RENDERER_NS_END

