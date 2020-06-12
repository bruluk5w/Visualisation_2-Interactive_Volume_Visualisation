#include "PitCollection.h"
#include "TextureResource.h"

#include <algorithm>

BRWL_RENDERER_NS


PitCollection::PitCollection() :
	tables {
		BRWL_CHAR_LITERAL("Refraction PIT"),
		BRWL_CHAR_LITERAL("Particle Color PIT"),
		BRWL_CHAR_LITERAL("Opacity PIT"),
		BRWL_CHAR_LITERAL("Medium Color PIT")
	}
{ }


PitCollection::~PitCollection()
{
	BRWL_EXCEPTION(
		!std::any_of(array, array + countof(array), [](const PitImage& p) { 
			return p.liveTexture->texture || p.stagedTexture->texture || p.fence || p.uploadEvent;
		}),
		BRWL_CHAR_LITERAL("Resources not properly destroyed")
	);
	tables.~Aliases();
}

bool PitCollection::init(ID3D12Device* device)
{
	for (int i = 0; i < countof(array); ++i)
	{
		PitImage& pit = array[i];
		if (!BRWL_VERIFY(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pit.fence))), BRWL_CHAR_LITERAL("Failed to create pitTexture fence.")))
		{
			destroy();
			return false;
		}

		pit.uploadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!BRWL_VERIFY(pit.uploadEvent != NULL, BRWL_CHAR_LITERAL("Failed to create upload event.")))
		{
			destroy();
			return false;
		}
	}

	return true;
}

bool PitCollection::isResident()
{
	return std::all_of(array, array + countof(array), [](const PitImage& p) {
		return p.liveTexture->state == TextureResource::State::RESIDENT;
	});
}

void PitCollection::destroy()
{
	for (int i = 0; i < countof(array); ++i)
	{
		PitImage& pit = array[i];
		
		pit.liveTexture->destroy();
		pit.stagedTexture->destroy();
		pit.fence = nullptr;
		if (pit.uploadEvent)
		{
			CloseHandle(pit.uploadEvent);
			pit.uploadEvent = NULL;
		}
	}
}


BRWL_RENDERER_NS_END

