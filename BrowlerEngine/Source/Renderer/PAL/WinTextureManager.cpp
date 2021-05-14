#include "PAL/WinTextureManager.h"

#ifdef BRWL_PLATFORM_WINDOWS

#include "BaseTexture.h"
#include "BaseTextureHandle.h"
#include "PAL/d3dx12.h"
#include "PAL/TextureResource.h"
#include "PAL/GpuTexture.h"
#include "PAL/WinTextureHandle.h"
#include "Renderer.h"


BRWL_RENDERER_NS

namespace PAL
{
	

	namespace
	{
		D3D12_RESOURCE_DIMENSION texDimToResourceDim[ENUM_CLASS_TO_NUM(TextureDimension::MAX)]
		{
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			D3D12_RESOURCE_DIMENSION_TEXTURE3D
		};

		D3D12_SRV_DIMENSION texDimToSrvDim[ENUM_CLASS_TO_NUM(TextureDimension::MAX)]
		{
			D3D12_SRV_DIMENSION_TEXTURE2D,
			D3D12_SRV_DIMENSION_TEXTURE3D
		};

		DXGI_FORMAT sampleFormatToDxgiFormat[ENUM_CLASS_TO_NUM(SampleFormat::MAX)]{
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_R32G32_UINT,
			DXGI_FORMAT_R16_SNORM,
			DXGI_FORMAT_R16_UNORM,
			DXGI_FORMAT_R32_SINT,
			DXGI_FORMAT_R32_UINT,
		};
	}


	WinTextureManager::WinTextureManager(ID3D12Device* device, DescriptorHeap* descriptorHeap, Renderer* renderer) :
		BaseTextureManager(),
		device(device),
		descriptorHeap(descriptorHeap),
		uploadCommandQueue(nullptr),
		uploadCommandAllocator(nullptr),
		uploadCommandList(nullptr),
		isCommandListReset(false),
		renderer(renderer),
		uploadSubmitted(),
		waitHandles(),
		promoted(false)
	{
		derivedThis = this;
	}

	WinTextureManager::~WinTextureManager()
	{
		for (GpuTexture* t : gpuTextures)
		{
			delete t;
		}
	}

	bool WinTextureManager::init()
	{
		this->destroy();

		{
			D3D12_COMMAND_QUEUE_DESC desc = {};
			desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 1;
			if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&uploadCommandQueue))), BRWL_CHAR_LITERAL("Failed to create upload command queue.")))
			{
				return false;
			}

			uploadCommandQueue->SetName(L"Upload Command Queue");
		}

		if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&uploadCommandAllocator))), BRWL_CHAR_LITERAL("Failed to create direct command allocator.")))
		{
			return false;
		}

		if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, uploadCommandAllocator.Get(), NULL, IID_PPV_ARGS(&uploadCommandList))), BRWL_CHAR_LITERAL("Failed to create upload command list.")))
		{
			return false;
		}

		if (!BRWL_VERIFY(SUCCEEDED(uploadCommandList->Close()), BRWL_CHAR_LITERAL("Failed to close upload command list.")))
		{
			return false;
		}

		isCommandListReset = false;

		return BaseTextureManager::init();
	}

	void WinTextureManager::destroy()
	{
		uploadCommandAllocator = nullptr;
		uploadCommandList = nullptr;
		uploadCommandQueue = nullptr;
	}

	void WinTextureManager::destroyAll()
	{
		std::scoped_lock l(registry.registryLock);
		if (!std::any_of(gpuTextures.begin(), gpuTextures.end(), [](const GpuTexture* p) { return p != nullptr && p->liveTexture != nullptr; }))
		{
			BRWL_CHECK(std::all_of(gpuTexIndex.begin(), gpuTexIndex.end(), [](const id_type i) { return i == BaseTextureHandle::Invalid.id; }), nullptr);
		}
		else
		{
			// todo: get rid of this and the renderer pointer member 
			// currently we have to do this because any texture might be currently used on the gpu;
			// we could also add it do a delete queue, and make sure in the destructor that the queue gets emptied before letting the texutre manager die
			renderer->waitForLastSubmittedFrame();
			for (GpuTexture* tex : gpuTextures)
			{
				if (tex != nullptr)
				{
					tex->destroy();
					delete tex;
				}
			}
		}

		gpuTextures.clear();
		gpuTexIndex.clear();

		BaseTextureManager::destroyAll(); 
	}

	void WinTextureManager::destroy(BaseTextureHandle& handle)
	{
		checkHandle(handle);
		if (handle.id < gpuTexIndex.size())
		{
			id_type& idx = gpuTexIndex[handle.id];
			GpuTexture* t = gpuTextures[idx];
			// In case destroy is called after some commands have been issued that are using this texture which is currently still uploading,
			// we have wait for the upload to finish.
			t->finishUpload();
			// todo: get rid of this and the renderer pointer member 
			// currently we have to do this because any texture might be currently used on the gpu;
			// we could also add it do a delete queue, and make sure in the destructor that the queue gets emptied before letting the texutre manager die
			renderer->waitForLastSubmittedFrame();

			// todo: additionally we have to put this texture on a delete queue until the dependent commands are done
			
			// then we can kill it
			t->destroy();
			idx = BaseTextureHandle::Invalid.id;
		}

		BaseTextureManager::destroy(handle);
	}

	bool WinTextureManager::startLoad(const BaseTextureHandle& handle)
	{
		std::scoped_lock l(registry.registryLock);
		id_type idx;
		BaseTexture* texture = get(handle, &idx);
		BRWL_EXCEPTION(texture->isValid(), BRWL_CHAR_LITERAL("Texture cannot be loaded to the GPU. It is invalid and will not have any data associated.")); // !BRWL_VERIFY(texture.	 == TextureResource::State::REQUESTING_UPLOAD, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))

		if (handle.id >= gpuTexIndex.size())
		{
			// TODO: replace data structure or improve growing behaviour
			// also currently the gpu resource index grows equally with the cpu texture registry. If there are many more cpu textures than textures promoted to gpu textures, then we might want to decouple this
			gpuTexIndex.resize(handle.id + 1, BaseTextureHandle::Invalid.id);
		}

		GpuTexture* gpuTex;
		if (gpuTexIndex[handle.id] == BaseTextureHandle::Invalid.id)
		{	// This texture is now on the GPU, assign new GPU texture
			// TODO: replace data structure or at least track next free GPU texture
			auto it = std::find_if(gpuTextures.begin(), gpuTextures.end(), [](const GpuTexture* t) { return t->liveTexture == nullptr; });
			if (it == gpuTextures.end())
			{	// create new GPU texture obect
				// TODO: replace data structure 
				gpuTexIndex[handle.id] = (decltype(gpuTexIndex)::value_type)gpuTextures.size();
				gpuTextures.push_back(new GpuTexture());
				gpuTex = gpuTextures.back();
			}
			else
			{	// reuse existing free GPU texture object
				gpuTexIndex[handle.id] = (decltype(gpuTexIndex)::value_type)(it - gpuTextures.begin());
				gpuTex = *it;
			}

			// create resources
			// TODO: we can make this more efficient by keeping some things in GpuTexture that are not tailored to one specific texture
			// such as the fence and upload event 
			gpuTex->init(this->device);
		}
		else
		{	// This texture is being updated and we have already a GPU texture object
			gpuTex = gpuTextures[gpuTexIndex[handle.id]];
			// todo: remove this line if not needed. do we have to wait for 
			BRWL_EXCEPTION(gpuTex->isReadyForUpload(), BRWL_CHAR_LITERAL("Texture "));
		}


		// TODO: encapsulate this in GpuTexture and TextureResource
		gpuTex->stagedTexture->texture = nullptr;
		gpuTex->stagedTexture->uploadHeap = nullptr;

		D3D12_RESOURCE_DESC texDesc;
		memset(&texDesc, 0, sizeof(texDesc));
		texDesc.Dimension = texDimToResourceDim[ENUM_CLASS_TO_NUM(texture->getDim())];
		texDesc.Alignment = 0;
		texDesc.Width = texture->getSizeX();
		texDesc.Height = texture->getSizeY();
		texDesc.DepthOrArraySize = texture->getSizeZ();
		texDesc.MipLevels = 1;
		texDesc.Format = sampleFormatToDxgiFormat[ENUM_CLASS_TO_NUM(texture->getSampleFormat())];
		texDesc.SampleDesc = { 1, 0 };
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // todo: try swizzled texture, may be faster
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// default heap
		CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&gpuTex->stagedTexture->texture)
		);

		if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for texture.")))
		{
			gpuTex->stagedTexture->texture = nullptr;
			gpuTex->stagedTexture->state = TextureResource::State::FAILED;
			return false;
		}

		gpuTex->stagedTexture->texture->SetName(texture->getName());

		uint64_t requiredSize = GetRequiredIntermediateSize(gpuTex->stagedTexture->texture.Get(), 0, 1);

		// upload heap
		// todo: could be deferred to create one in update() for the whole frame or
		// better create one at the beginning and reuse
		heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);
		hr = device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&gpuTex->stagedTexture->uploadHeap)
		);

		if (!BRWL_VERIFY(SUCCEEDED(hr), BRWL_CHAR_LITERAL("Failed to create committed resource for the volume texture upload heap.")))
		{
			gpuTex->stagedTexture->texture = nullptr;
			gpuTex->stagedTexture->uploadHeap = nullptr;
			gpuTex->stagedTexture->state = TextureResource::State::FAILED;
			return false;
		}

		gpuTex->stagedTexture->uploadHeap->SetName(BRWL_CHAR_LITERAL("Texture Upload Heap"));

		gpuTex->requestUpload();

		return true;
	}

	bool WinTextureManager::isResident(const BaseTextureHandle& handle) const
	{
		checkTextureId(handle.id);
		std::scoped_lock l(registry.registryLock);
		if (handle.id < gpuTexIndex.size())
		{
			const id_type idx = gpuTexIndex[handle.id];
			if (idx != BaseTextureHandle::Invalid.id && idx >= 0 && idx < gpuTextures.size())
			{
				return gpuTextures[idx]->isResident();
			}
		}

		return false;
	}

	//todo: get the logger into these HandleDeviceRemoved calls
	bool WinTextureManager::update()
	{
		std::scoped_lock l(registry.registryLock);
		if (!isCommandListReset) {
			PAL::HandleDeviceRemoved(
				uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr),
				renderer->getDevice()
			);
			isCommandListReset = true;
		}

		uploadSubmitted.clear();
		for (id_type id = 0; id < gpuTexIndex.size(); ++id)
		{
			const id_type idx = gpuTexIndex[id];
			if (idx == BaseTextureHandle::Invalid.id)
				continue;

			GpuTexture* t = gpuTextures[idx];

			if (!t || !t->stagedTexture || t->stagedTexture->state != TextureResource::State::REQUESTING_UPLOAD)
				continue;

			BaseTexture* texture = registry.store[registry.index[id]];

			// TODO: encapsulate this in GpuTexture and TextureResource
			D3D12_SUBRESOURCE_DATA textureData{
				texture->getPtr(),
				texture->getStrideY(),
				texture->getStrideZ()
			};

			// TODO: see if things can be optimized by removing allocating d3dx12 helper
			uint64_t result = UpdateSubresources(
				this->uploadCommandList.Get(),
				t->stagedTexture->texture.Get(),
				t->stagedTexture->uploadHeap.Get(),
				0, // upload heap offset
				0, // first subresource
				1, // num subresources
				&textureData
			);

			if (!BRWL_VERIFY(result != 0, BRWL_CHAR_LITERAL("Failed to upload texture data.")))
			{
				t->stagedTexture->texture = nullptr;
				t->stagedTexture->uploadHeap = nullptr;
				t->stagedTexture->state = TextureResource::State::FAILED;
				continue;
			}

			// Create a SRV for the texture
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = sampleFormatToDxgiFormat[ENUM_CLASS_TO_NUM(texture->getSampleFormat())];;
			srvDesc.ViewDimension = texDimToSrvDim[ENUM_CLASS_TO_NUM(texture->getDim())];
			switch (texture->getDim())
			{
			case TextureDimension::TEXTURE_2D:
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.PlaneSlice = 0;
				break;
			case TextureDimension::TEXTURE_3D:
				srvDesc.Texture3D.MipLevels = 1;
				break;
			default:
				BRWL_UNREACHABLE();
				break;
			}

			if (!t->stagedTexture->descriptorHandle)
				t->stagedTexture->descriptorHandle = this->descriptorHeap->allocateOne(texture->getName());

			device->CreateShaderResourceView(t->stagedTexture->texture.Get(), &srvDesc, t->stagedTexture->descriptorHandle->getNonResident().cpu);

			uploadSubmitted.push_back(idx);

			t->stagedTexture->state = TextureResource::State::LOADING;
		}

		if (!uploadSubmitted.empty())
		{
			PAL::HandleDeviceRemoved(
				uploadCommandList->Close(),
				renderer->getDevice()
			);
			ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
			uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);
			isCommandListReset = false;

			for (id_type idx : uploadSubmitted)
			{
				GpuTexture* tex = gpuTextures[idx];
				++tex->uploadFenceValue;

				PAL::HandleDeviceRemoved(
					uploadCommandQueue->Signal(tex->fence.Get(), tex->uploadFenceValue),
					renderer->getDevice()
				);
			}
		}

		return !uploadSubmitted.empty();
	}

	bool WinTextureManager::promoteStagedTextures()
	{
		std::scoped_lock l(registry.registryLock);

		promoteStagedTexturesInternal();

		bool didPromote = promoted;
		promoted = false;
		return didPromote;
	}

	void WinTextureManager::waitForPendingUploads(BaseTextureHandle* handles, id_type numHandles)
	{
		std::scoped_lock l(registry.registryLock);
#ifdef _DEBUG
		BRWL_EXCEPTION(descriptorHeap->isFrameActive(), BRWL_CHAR_LITERAL("This method may only be called within an active resource frame."));
#endif
		BRWL_EXCEPTION(waitHandles.empty(), nullptr); // gathers all events we still have to wait for
		for (id_type i = 0; i < numHandles; ++i)
		{
			BaseTextureHandle* handle = handles[i].asPlatformHandle();
			checkTextureId(handle->id);
			if (handle->id >= gpuTexIndex.size())
				continue;

			const id_type idx = gpuTexIndex[handle->id];

			if (idx == BaseTextureHandle::Invalid.id || idx < 0 || idx >= gpuTextures.size(), nullptr)
				continue;

			GpuTexture* tex = gpuTextures[idx];
			if (!tex->isReadyForUpload())
			{
				// if the staged texture is currently upoading then the fence value has do be lower than the one which we remembered
				// but only do a weak check since it could theoretically finish in this very moment
				uint64_t completedValue = tex->fence->GetCompletedValue();
				BRWL_CHECK(completedValue <= tex->uploadFenceValue, BRWL_CHAR_LITERAL("Invalid fence/staged texture state."));
				uint64_t x = tex->fence->GetCompletedValue(); // helpful for debugging to see if it actually uploaded now
				if (!BRWL_VERIFY(!tex->isFailed(), BRWL_CHAR_LITERAL("Invalid state for staged texture.")))
				{
					continue;
				}

				// prepare for waiting 
				tex->fence->SetEventOnCompletion(tex->uploadFenceValue, tex->uploadEvent);
				waitHandles.push_back(tex->uploadEvent);
			}
		}

		// wait and clean up event state 
		if (waitHandles.size())
		{
			WaitForMultipleObjects(waitHandles.size(), waitHandles.data(), true, INFINITE);
			for (const HANDLE& handle : waitHandles)
			{
				ResetEvent(handle);
			}

			waitHandles.clear();

			promoteStagedTexturesInternal(); // swap textures
		}
	}

	const DescriptorHandle* WinTextureManager::getDescriptorHandle(const WinTextureHandle& handle) const
	{
		checkTextureId(handle.id);
		std::scoped_lock l(registry.registryLock);
		id_type idx = getGpuIndex(handle.id);

		GpuTexture* tex = gpuTextures[gpuTexIndex[idx]];
		BRWL_EXCEPTION(tex->isResident(), BRWL_CHAR_LITERAL("Trying to retrieve descriptor for non-resident texture"));
		return tex->liveTexture->descriptorHandle;
	}

	bool WinTextureManager::isReadyForUpload(const WinTextureHandle& handle) const
	{
		checkTextureId(handle.id);
		std::scoped_lock l(registry.registryLock);
		id_type idx = getGpuIndex(handle.id);
		if (idx >= gpuTexIndex.size())
		{
			return false;
		}

		return gpuTextures[gpuTexIndex[idx]]->isReadyForUpload();
	}

	ID3D12Resource* WinTextureManager::getLiveResource(const WinTextureHandle& handle)
	{
		checkTextureId(handle.id);
		std::scoped_lock l(registry.registryLock);
		id_type idx = getGpuIndex(handle.id);

		GpuTexture* tex = gpuTextures[gpuTexIndex[idx]];
		BRWL_EXCEPTION(tex->isResident(), BRWL_CHAR_LITERAL("Invalid state!"));

		return tex->liveTexture->texture.Get();
	}


	void WinTextureManager::promoteStagedTexturesInternal()
	{
#ifdef _DEBUG
		BRWL_EXCEPTION(descriptorHeap->isFrameActive(), BRWL_CHAR_LITERAL("This method may only be called within an active resource frame."));
#endif
		for (id_type id = 0; id < gpuTexIndex.size(); ++id)
		{
			const id_type idx = gpuTexIndex[id];
			if (idx == BaseTextureHandle::Invalid.id)
				continue;

			GpuTexture* t = gpuTextures[idx];

			if (!t || !t->stagedTexture || t->stagedTexture->state != TextureResource::State::LOADING)
				continue;

			if (!t->isUploading())
			{
				// We might still have some frames in flight with the old texture
				// We have to wait until we are sure that we are the only one tampering with resources
				// todo: instead of this, add texture to a delete queue defer deletion to when the staged texture has to be reused and check for dependent commands
				renderer->waitForLastSubmittedFrame();
				// swap staged to live
				t->stagedTexture->state = TextureResource::State::RESIDENT;
				t->stagedTexture->uploadHeap = nullptr; // free some resources
				std::swap(t->stagedTexture, t->liveTexture);
				// release old texture
				t->stagedTexture->destroy();
				// Indicate new resource to be used by a compute shader.
				// todo: optimize by calling just once after loop with multiple barriers
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(t->liveTexture->texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				renderer->getCommandList()->ResourceBarrier(1, &barrier);
				// todo: add mechanism for application to check which textures have been updated/become resident since the last frame
				//hasViewChanged = true;
				promoted = true;
			}
		}
	}

	BaseTextureManager::id_type WinTextureManager::getGpuIndex(const id_type id) const
	{
		BRWL_EXCEPTION(id < gpuTexIndex.size(), BRWL_CHAR_LITERAL("Texture ID out of bounds."));
		const id_type idx = gpuTexIndex[id];
		BRWL_CHECK(idx != BaseTextureHandle::Invalid.id && idx >= 0 && idx < gpuTextures.size(), nullptr);
		return idx;
	}

} // namespace PAL

BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS
