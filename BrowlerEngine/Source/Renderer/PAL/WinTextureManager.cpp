#include "PAL/WinTextureManager.h"

#include "BaseTexture.h"
#include "Renderer/PAL/d3dx12.h"
#include "Renderer.h"

#include <algorithm>
#include <array>

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS


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

	// todo: remove if not necessary
	//template<SampleFormat S> struct WinSampleFormatTrait : public SampleFormatTrait<S> {
	//	// some type trait prevents the static assertion from being triggered
	//	static_assert(S && false, "No DXGI_FORMAT found for sample type enum.");
	//};

	//template<> struct WinSampleFormatTrait<SampleFormat::F32> {
	//	static const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32_FLOAT;
	//};

	//template<> struct WinSampleFormatTrait<SampleFormat::F64> {
	//	static const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_UINT;
	//};

	//template<> struct WinSampleFormatTrait<SampleFormat::S16> {
	//	static const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R16_SINT;
	//};

	//template<> struct WinSampleFormatTrait<SampleFormat::U16> {
	//	static const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R16_UINT;
	//};

	//template<> struct WinSampleFormatTrait<SampleFormat::S32> {
	//	static const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32_SINT;
	//};

	//template<> struct WinSampleFormatTrait<SampleFormat::U32> {
	//	static const DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32_UINT;
	//};

	DXGI_FORMAT sampleFormatToDxgiFormat[ENUM_CLASS_TO_NUM(SampleFormat::MAX)]{
		DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_UINT,
	};
}

namespace PAL
{
	void WinTextureManager::destroyAll()
	{
		std::scoped_lock l(registry.registryLock);
		if (!std::any_of(gpuTextures.begin(), gpuTextures.end(), [](const GpuTexture* p) { return p != nullptr && p->liveTexture != nullptr; }))
		{
			BRWL_CHECK(std::all_of(gpuTexIndex.begin(), gpuTexIndex.end(), [](const id_type i) { return i == TextureHandle::Invalid.id; }), nullptr);
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

	void WinTextureManager::destroy(TextureHandle& handle)
	{
		checkHandle(handle);
		if (handle.id < gpuTexIndex.size())
		{
			id_type& idx = gpuTexIndex[handle.id];
			GpuTexture* t = gpuTextures[idx];
			// In case destroy is called after some commands have been issued that are using this texture which is currently still uploading,
			// we have wait for the upload to finish.
			t->waitForUploads();
			// todo: get rid of this and the renderer pointer member 
			// currently we have to do this because any texture might be currently used on the gpu;
			// we could also add it do a delete queue, and make sure in the destructor that the queue gets emptied before letting the texutre manager die
			renderer->waitForLastSubmittedFrame();

			// todo: additionally we have to put this texture on a delete queue until the dependent commands are done
			
			// then we can kill it
			t->destroy();
			idx = TextureHandle::Invalid.id;
		}

		BaseTextureManager::destroy(handle);
	}

	bool WinTextureManager::startLoad(const TextureHandle& handle)
	{
		std::scoped_lock l(registry.registryLock);
		id_type idx;
		BaseTexture* texture = get(handle, &idx);
		BRWL_EXCEPTION(!texture->isValid(), BRWL_CHAR_LITERAL("Texture cannot be loaded to the GPU. It is invalid and will not have any data associated.")); // !BRWL_VERIFY(texture.	 == TextureResource::State::REQUESTING_UPLOAD, BRWL_CHAR_LITERAL("Invalid texture resouce state.")))

		if (handle.id < gpuTexIndex.size())
		{
			// TODO: replace data structure or improve growing behaviour
			// also currently the gpu resource index grows equally with the cpu texture registry. If there are many more cpu textures than textures promoted to gpu textures, then we might want to decouple this
			gpuTexIndex.resize(handle.id + 20, TextureHandle::Invalid.id);
		}

		GpuTexture* gpuTex;
		if (gpuTexIndex[handle.id] == TextureHandle::Invalid.id)
		{	// This texture is now on the GPU, assign new GPU texture
			// TODO: replace data structure or at least track next free GPU texture
			auto it = std::find_if(gpuTextures.begin(), gpuTextures.end(), [](const GpuTexture* t) { return t->liveTexture == nullptr; });
			if (it == gpuTextures.end())
			{	// create new GPU texture obect
				// TODO: replace data structure 
				gpuTextures.push_back(new GpuTexture());
				gpuTex = gpuTextures.back();
			}
			else
			{	// reuse existing free GPU texture object
				gpuTex = *it;
			}

			// create resources
			// TODO: we can make this more efficient by keeping some things in GpuTexture that are not tailored to one specific texture
			// such as the fence and upload event 
			gpuTex->init(this->device);
			gpuTexIndex[handle.id] = it - gpuTextures.begin();
		}
		else
		{	// This texture is being updated and we have already a GPU texture object
			gpuTex = gpuTextures[gpuTexIndex[handle.id]];
			// todo: remove this line
			//BRWL_EXCEPTION(gpuTex->isReadyForUpload(), BRWL_CHAR_LITERAL("Texture "));
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

		gpuTex->requestUpload();

		return true;
	}



	bool WinTextureManager::isResident(const TextureHandle& handle) const
	{
		checkTextureId(handle.id);
		std::scoped_lock l(registry.registryLock);
		id_type idx = getIndex(handle.id);
		if (idx >= gpuTexIndex.size())
		{
			return false;
		}

		return gpuTextures[gpuTexIndex[idx]]->isResident();
	}

	bool WinTextureManager::update()
	{
		std::scoped_lock l(registry.registryLock);
		id_type id = 0;
		for(id_type idx : gpuTexIndex)
		{
			if (idx == TextureHandle::Invalid.id)
				continue;

			GpuTexture* t = gpuTextures[idx];

			if (!t || !t->stagedTexture || t->stagedTexture->state != TextureResource::State::REQUESTING_UPLOAD)
				continue;

			BaseTexture* texture = registry.store[registry.index[id]];

			// TODO: encapsulate this in GpuTexture and TextureResource
			D3D12_SUBRESOURCE_DATA textureData {
				texture->getPtr(),
				texture->getStrideY(),
				texture->getBufferSize()
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
				return false;
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

			device->CreateShaderResourceView(t->stagedTexture->texture.Get(), &srvDesc, t->stagedTexture->descriptorHandle->getNonResident().cpu);

			t->stagedTexture->state = TextureResource::State::LOADING;
			return true;

			where does the descriptor come from?

		// we always need the volume texture so ensure we have it first.
		// this stalls the pipeline until the copy is complete but maybe we don't care 
		// because this is the data we want to visualize and if we don't have it we see nothing anyways
		// if (volumeTexture.state == TextureResource::State::REQUESTING_UPLOAD)
		// {
		//     BRWL_EXCEPTION(dataSet.isValid(), BRWL_CHAR_LITERAL("Invalid state of data set."));
		//     uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);

		  // Since the volume texture is not double buffered, we have to wait unil all frames still rendering from the volume texture have finished
	//    r->waitForLastSubmittedFrame();
	//
	//    volumeTexture.descriptorHandle = r->srvHeap.allocateOne(BRWL_CHAR_LITERAL("VolumeTexture"));
	//    if (!BRWL_VERIFY(LoadVolumeTexture(r->device.Get(), uploadCommandList.Get(), &dataSet, volumeTexture), BRWL_CHAR_LITERAL("Failed to load the volume texture to the GPU.")))
	//    {   // we expect the function to clean up everything necessary
	//        return;
	//    };
	//
	//    DR(uploadCommandList->Close());
	//    ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
	//    uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);
	//
	//    // wait synchronously for the upload to complete
	//    ++volumeTextureFenceValue;
	//    DR(uploadCommandQueue->Signal(volumeTextureUploadFence.Get(), volumeTextureFenceValue));
	//    DR(volumeTextureUploadFence->SetEventOnCompletion(volumeTextureFenceValue, uploadFenceEvent));
	//    WaitForSingleObject(uploadFenceEvent, INFINITE);
	//    volumeTexture.state = TextureResource::State::RESIDENT;
	//    volumeTexture.uploadHeap = nullptr;  // free resources
	//
	//    // indicate new resource to be used by a compute shader
	//    r->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(volumeTexture.texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	//    hasViewChanged = true;
	//    //}
	//    {
	//        bool descriptorHeapChanged = false;
	//        uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);
	//        std::scoped_lock(registryAccess);
	//        for (BaseTexture* const tex : registry)
	//        {
	//            if (!tex) continue;
	//            if (tex->gpu.stagedTexture->state == TextureResource::State::REQUESTING_UPLOAD)
	//            {
	//                BRWL_EXCEPTION(tex->isValid(), BRWL_CHAR_LITERAL("Texture is requesting upload but has no CPU buffer set."));
	//
	//            }
	//        }
	//    }
	//
	//}
	//
	//void TextureManager::waitForPendingUploads(BaseTexture**, int count)
	//{
	//    // (wait for ongoing upload to finish +) request upload
	////-------------------
	//
	//// Check if textures should be recomputed
	//    bool mustRecompute[countof(*((decltype(pitCollection.array)*)nullptr))] = { false };
	//    bool blocked[countof(mustRecompute)] = { false };
	//    for (int i = 0; i < countof(pitCollection.array); ++i)
	//    {
	//        PitImage& pitImage = pitCollection.array[i];
	//
	//        mustRecompute[i] = true;
	//        // If staged texture is not free, wait for upload to complete
	//        if (pitImage.stagedTexture->state != TextureResource::State::NONE)
	//        {
	//            blocked[i] = true;
	//            // if the staged resource is currently upoading then the fence value has do be lower than the one which we remembered
	//            // but only do a weak check since it could theoretically finish in this very moment
	//            uint64_t completedValue = pitImage.fence->GetCompletedValue();
	//            BRWL_CHECK(completedValue <= pitImage.uploadFenceValue, BRWL_CHAR_LITERAL("Invalid fence/staged resource state."));
	//            uint64_t x = pitImage.fence->GetCompletedValue();
	//            if (!BRWL_VERIFY(pitImage.stagedTexture->state != TextureResource::State::FAILED, BRWL_CHAR_LITERAL("Invalid state for staged pitTexture.")))
	//            {
	//                continue;
	//            }
	//
	//            // prepare for waiting 
	//            pitImage.fence->SetEventOnCompletion(pitImage.uploadFenceValue, pitImage.uploadEvent);
	//        }
	//    }
	//}
	//
	//// gather all events we still have to wait for
	//HANDLE waitHandles[countof(blocked)] = { NULL };
	//unsigned int numWaitHandles = 0;
	//for (int i = 0; i < countof(pitCollection.array); ++i)
	//{
	//    if (blocked[i])
	//    {
	//        PitImage& pitImage = pitCollection.array[i];
	//        waitHandles[numWaitHandles] = pitImage.uploadEvent;
	//        ++numWaitHandles;
	//    }
	//}
	//
	//// wait and clean up event state 
	//if (numWaitHandles)
	//{
	//    WaitForMultipleObjects(numWaitHandles, waitHandles, true, INFINITE);
	//}
	//
	//for (int i = 0; i < countof(pitCollection.array); ++i)
	//{
	//    if (blocked[i])
	//    {
	//        PitImage& pitImage = pitCollection.array[i];
	//        ResetEvent(pitImage.uploadEvent);
	//        // Since the live texture is currently in use in the front-end we do not swap but immediately recycle the staged texture.
	//        // This means that a texture change is only visible to the user if there is one frame without a change that requires recomputation.
	//        pitImage.stagedTexture->destroy();
	//    }
	//}
	//	return true;
	//}

	void WinTextureManager::promoteStagedTextures()
	{
		for (int i = 0; i < countof(pitCollection.array); ++i)
		{
			PitImage& pitImage = pitCollection.array[i];
			const uint64_t completed = pitImage.fence->GetCompletedValue();

			// swap texture if ready
			if (pitImage.stagedTexture->state == TextureResource::State::LOADING && completed >= pitImage.uploadFenceValue)
			{
				pitImage.stagedTexture->state = TextureResource::State::RESIDENT;
				pitImage.stagedTexture->uploadHeap = nullptr; // free some resources
				// We might still have some frames in flight with the old texture
				// We have to wait until we are sure we that are the only one tampering with resources
				renderer->waitForLastSubmittedFrame();
				std::swap(pitImage.stagedTexture, pitImage.liveTexture);
				// release old texture
				pitImage.stagedTexture->destroy();
				// Indicate new resource to be used by a compute shader.
				renderer->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(pitImage.liveTexture->texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
				hasViewChanged = true;
			}
		}
	}







} // namespace PAL


BRWL_RENDERER_NS_END

#endif // BRWL_PLATFORM_WINDOWS