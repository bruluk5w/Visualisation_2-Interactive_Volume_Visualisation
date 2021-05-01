#include "PAL/GpuTexture.h"

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS


namespace PAL
{

	GpuTexture::~GpuTexture()
	{
		BRWL_EXCEPTION(!liveTexture && !stagedTexture && !fence && !uploadEvent,
			BRWL_CHAR_LITERAL("Resources have not been properly destroyed.")
		);
	}

	bool GpuTexture::init(ID3D12Device* device)
	{
		if (!BRWL_VERIFY(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))), BRWL_CHAR_LITERAL("Failed to create texture fence.")))
		{
			destroy();
			return false;
		}

		uploadFenceValue = 0;

		uploadEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual reset TRUE
		if (!BRWL_VERIFY(uploadEvent != NULL, BRWL_CHAR_LITERAL("Failed to create texture upload event.")))
		{
			destroy();
			return false;
		}

		liveTexture = std::make_unique<TextureResource>();
		stagedTexture = std::make_unique<TextureResource>();

		return true;
	}

	void GpuTexture::destroy()
	{
		if (liveTexture)
		{
			liveTexture->destroy();
			liveTexture = nullptr;
		}
		if (stagedTexture)
		{
			stagedTexture->destroy();
			stagedTexture = nullptr;

		}

		uploadFenceValue = 0;
		fence = nullptr;

		if (uploadEvent)
		{
			CloseHandle(uploadEvent);
			uploadEvent = NULL;
		}
	}

	bool GpuTexture::isResident() const
	{
		return liveTexture && liveTexture->isResident();
	}


	void GpuTexture::requestUpload()
	{
		BRWL_EXCEPTION(isReadyForUpload(), BRWL_CHAR_LITERAL("TextureResource was not in the expected state."));
		stagedTexture->state = TextureResource::State::REQUESTING_UPLOAD;
	}


	bool GpuTexture::isReadyForUpload()
	{
		return stagedTexture->state == TextureResource::State::NONE;
	}

}


BRWL_RENDERER_NS_END

#endif
