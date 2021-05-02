#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS


#include "TextureResource.h"
#include "BaseTexture.h"

BRWL_RENDERER_NS


namespace PAL
{


	// A double buffered GPU texure resource
	struct GpuTexture
	{
		GpuTexture() :
			fence(nullptr),
			uploadFenceValue(0),
			uploadEvent(NULL),
			liveTexture(nullptr),
			stagedTexture(nullptr)
		{ }

		virtual ~GpuTexture();
		/*!
		 * Creates the respective graphics resources required to upload and use the texture on the GPU.
		 * \param device The device to create the resource for.
		 * \return Returns true if the initialization was successful, else false.
		 */
		bool init(ID3D12Device* device);
		/*!
		 * Frees all internal resources and resets the GpuTexture to its initial state before any init(...) call.
		 */
		void destroy();

		bool isResident() const;
		bool isReadyForUpload() const;
		void requestUpload();
		bool isUploading() const;
		void waitForUploads();

		ComPtr<ID3D12Fence> fence;
		uint64_t uploadFenceValue;
		HANDLE uploadEvent;
		std::unique_ptr<TextureResource> liveTexture;
		std::unique_ptr<TextureResource> stagedTexture;
	};


}


BRWL_RENDERER_NS_END


#endif
