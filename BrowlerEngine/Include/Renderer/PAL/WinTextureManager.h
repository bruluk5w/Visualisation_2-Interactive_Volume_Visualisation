#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

#include "BaseTextureManager.h"
#include "RendererFwd.h"

BRWL_RENDERER_NS


namespace PAL
{
	// All texture manager methods available through the base texture handle are listed here (return type, name, (arguments), const)
#define FOR_EACH_WIN_TEXTURE_HANDLE_METHOD(f) \
	f(const DescriptorHandle*, getDescriptorHandle, (), const) \
	f(bool, isReadyForUpload, (), const) \
	f(ID3D12Resource*, getLiveResource, ())

	class DescriptorHeap;
	class DescriptorHandle;
	struct GpuTexture;
	struct WinTextureHandle;

	class WinTextureManager : public BaseTextureManager
	{
	public:
		WinTextureManager(ID3D12Device* device, DescriptorHeap* descriptorHeap, Renderer* renderer);

		virtual ~WinTextureManager();

		ID3D12Device* getDevice() const { return device; }

		//! Remove and destroy all textures managed by the texture manager
		virtual void destroyAll() override;

		//! Remove and destroy the texture of this handle
		virtual void destroy(BaseTextureHandle& handle) override;

		//! Dispatch loading the texture
		/**
		 * Implicitly initializes GPU resources if they don't exist yet.
		 * Note: As long as committed resources are used in the implementation this should ideally be called form a separate thread
		 * two heaps are implicitly crated (upload and default (in non-cpu accessible memory)).
		 * These operations are blocking until the resource heap is resident.
		 */
		virtual bool startLoad(const BaseTextureHandle& handle) override;

		//! Returns true if the GPU resources are resident in GPU memory
		virtual bool isResident(const BaseTextureHandle& handle) const override;

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
		virtual bool promoteStagedTextures() override;

		//! Blocks until a staged texture becomes available to the CPU.
		virtual void waitForPendingUploads(BaseTextureHandle* handles, id_type numHandles) override;

#pragma region Texture handle methods
		//! Get descriptor handle of live texture.
		/**
		 * May only be called if the texture is resident (only then it has a descriptor handle for the live GPU texture.
		 */
		const DescriptorHandle* getDescriptorHandle(const WinTextureHandle& handle) const;

		//! Returns true if there is a staged texture that is currently not uploading
		bool isReadyForUpload(const WinTextureHandle& handle) const;

		//! Returns DX12 live resource for this handle, provided it exists.
		ID3D12Resource* getLiveResource(const WinTextureHandle& handle);

#pragma endregion Texture handle methods

	protected:
		void promoteStagedTexturesInternal();
		id_type getGpuIndex(const id_type id) const;

		ID3D12Device* device;
		DescriptorHeap* descriptorHeap;
		ComPtr<ID3D12CommandQueue> uploadCommandQueue;
		ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
		ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
		bool isCommandListReset; // whether or not the current command list has been reset

		std::vector<GpuTexture*> gpuTextures;		
		std::vector<id_type> gpuTexIndex;
		// todo: get rid of this pointer by tracking when a texture is used and checking if the last usage is already through the pipeline.
		// then we do not have to call a wait method on the renderer but can decide per each texture if it is referenced by any frame that is current in flight.
		Renderer* renderer;
	private:
		std::vector<id_type> uploadSubmitted; //!< buffer of indices used in update() to keep track of updated textures
		std::vector<HANDLE> waitHandles; //!< buffer for os event handles to wait on in waitForPendingUploads
		bool promoted;
	};


}


BRWL_RENDERER_NS_END


#endif // BRWL_PLATFORM_WINDOWS
