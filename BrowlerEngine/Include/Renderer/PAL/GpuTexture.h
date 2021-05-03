#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS


class BaseTexture;

namespace PAL
{


	struct TextureResource;

	// A double buffered GPU texure resource
	struct GpuTexture
	{
		GpuTexture();

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
		bool isUploading() const;
		bool isFailed() const;
		void requestUpload();
		void finishUpload();

		ComPtr<ID3D12Fence> fence;
		uint64_t uploadFenceValue;
		HANDLE uploadEvent;
		std::unique_ptr<TextureResource> liveTexture;
		std::unique_ptr<TextureResource> stagedTexture;
	};


} // namespace PAL

BRWL_RENDERER_NS_END


#endif
