#pragma once

#ifdef BRWL_PLATFORM_WINDOWS


#include "BaseTextureManager.h"
#include "GpuTexture.h"
#include "RendererFwd.h"

BRWL_RENDERER_NS


namespace PAL
{


	class DescriptorHeap;

	class WinTextureManager : public BaseTextureManager
	{
	public:
		WinTextureManager(ID3D12Device* device, DescriptorHeap* descriptorHeap, Renderer* renderer) :
			BaseTextureManager(),
			device(device),
			descriptorHeap(descriptorHeap),
			uploadCommandQueue(nullptr),
			uploadCommandAllocator(nullptr),
			uploadCommandList(nullptr),
			renderer(renderer)
		{ }

		virtual ~WinTextureManager()
		{
			for (GpuTexture* t : gpuTextures)
			{
				delete t;
			}
		}

		ID3D12Device* getDevice() const { return device; }

		//! Remove and destroy all textures managed by the texture manager
		virtual void destroyAll() override;

		//! Remove and destroy the texture of this handle
		virtual void destroy(TextureHandle& handle) override;

		//! Dispatch loading the texture
		/**
		 * Implicitly initializes GPU resources if they don't exist yet.
		 * Note: As long as committed resources are used in the implementation this should ideally be called form a separate thread
		 * two heaps are implicitly crated (upload and default (in non-cpu accessible memory)).
		 * These operations are blocking until the resource heap is resident.
		 */
		virtual bool startLoad(const TextureHandle& handle) override;
		//! Returns true if the GPU resources are resident in GPU memory
		virtual bool isResident(const TextureHandle& handle) const override;

		//! Updates the textures managed by the TextureManager
		/**
		 * Updates the textures that request upload and swaps the staging textures with the live textures in case they are ready.
		 * \return True if the the TextureManager touched any of the Textures, potentially causing descriptors to be modified. In this case all descriptor heaps that contain descriptors of the managed textures need an update.
		 */
		 virtual bool update() override;
		 //! Promotes staged textures to live textures
		 /**
		  * Swaps pointers to staged textures with pointers to live textures if staged textures are resident.
		  */
		 virtual void promoteStagedTextures() override;
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
		// todo: get rid of this pointer by tracking when a texture is used and checking if the last usage is already through the pipeline.
		// then we do not have to call a wait method on the renderer but can decide per each texture if it is referenced by any frame that is current in flight.
		Renderer* renderer; 
	};


}


BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS
