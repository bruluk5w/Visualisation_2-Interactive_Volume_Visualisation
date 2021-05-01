#pragma once

#ifdef BRWL_PLATFORM_WINDOWS


#include "BaseTextureManager.h"
#include "GpuTexture.h"


BRWL_RENDERER_NS


namespace PAL
{


	class DescriptorHeap;

	class WinTextureManager : public BaseTextureManager
	{
	public:
		WinTextureManager(ID3D12Device* device, DescriptorHeap* descriptorHeap) :
			BaseTextureManager(),
			device(device),
			descriptorHeap(descriptorHeap),
			uploadCommandQueue(nullptr),
			uploadCommandAllocator(nullptr),
			uploadCommandList(nullptr)
		{ }

		virtual ~WinTextureManager()
		{
			for (GpuTexture* t : gpuTextures)
			{
				delete t;
			}
		}

		ID3D12Device* getDevice() const { return device; }

		//! Initialize GPU resources for this texture and dispatch loading
		virtual bool load(const TextureHandle& handle) override;
		//! Returns true if the GPU resources are resident in GPU memory
		virtual bool isResident(const TextureHandle& handle) const override;

		//! Updates the textures managed by the TextureManager
		/**
		 * Updates the textures that request upload and swaps the staging textures with the live textures in case they are ready.
		 * \return True if the the TextureManager touched any of the Textures, potentially causing descriptors to be modified. In this case all descriptor heaps that contain descriptors of the managed textures need an update.
		 */
		 virtual bool update() override;
		 //! 
		 //void waitForPendingUploads();

	protected:
		ID3D12Device* device;
		DescriptorHeap* descriptorHeap;
		ComPtr<ID3D12CommandQueue> uploadCommandQueue;
		ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> uploadCommandList;

		std::vector<GpuTexture*> gpuTextures;
		std::vector<id_type> gpuTexIndex;
	};


}


BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS
