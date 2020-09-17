#include "TextureManager.h"

BRWL_RENDERER_NS


void TextureManager::add(BaseTexture* tex)
{
	std::scoped_lock(registryAccess);
	BRWL_EXCEPTION(std::find(registry.begin(), registry.end(), tex) == registry.end(), BRWL_CHAR_LITERAL("Cannot add the same texture twice."));
	const auto it = std::find(registry.begin(), registry.end(), nullptr);
	if (it == registry.end()) {
		registry.push_back(tex);
		return;
	}

	registry[it - registry.begin()] = tex;

}

void TextureManager::remove(BaseTexture* tex)
{
	std::scoped_lock(registryAccess);
	const auto it = std::find(registry.begin(), registry.end(), tex);
	if (!BRWL_VERIFY(it != registry.end(), BRWL_CHAR_LITERAL("The texture which is tried to be removed is not present.")))
	{
		return;
	}

	registry[it - registry.begin()] = nullptr;
}

bool TextureManager::update()
{
    // we always need the volume texture so ensure we have it first.
   // this stalls the pipeline until the copy is complete but maybe we don't care 
   // because this is the data we want to visualize and if we don't have it we see nothing anyways
    if (volumeTexture.state == TextureResource::State::REQUESTING_UPLOAD)
    {
        BRWL_EXCEPTION(dataSet.isValid(), BRWL_CHAR_LITERAL("Invalid state of data set."));
        uploadCommandList->Reset(uploadCommandAllocator.Get(), nullptr);

        // Since the volume texture is not double buffered, we have to wait unil all frames still rendering from the volume texture have finished
        r->waitForLastSubmittedFrame();

        volumeTexture.descriptorHandle = r->srvHeap.allocateOne(BRWL_CHAR_LITERAL("VolumeTexture"));
        if (!BRWL_VERIFY(LoadVolumeTexture(r->device.Get(), uploadCommandList.Get(), &dataSet, volumeTexture), BRWL_CHAR_LITERAL("Failed to load the volume texture to the GPU.")))
        {   // we expect the function to clean up everything necessary
            return;
        };

        DR(uploadCommandList->Close());
        ID3D12CommandList* const ppCommandLists[] = { uploadCommandList.Get() };
        uploadCommandQueue->ExecuteCommandLists(1, ppCommandLists);

        // wait synchronously for the upload to complete
        ++volumeTextureFenceValue;
        DR(uploadCommandQueue->Signal(volumeTextureUploadFence.Get(), volumeTextureFenceValue));
        DR(volumeTextureUploadFence->SetEventOnCompletion(volumeTextureFenceValue, uploadFenceEvent));
        WaitForSingleObject(uploadFenceEvent, INFINITE);
        volumeTexture.state = TextureResource::State::RESIDENT;
        volumeTexture.uploadHeap = nullptr;  // free resources

        // indicate new resource to be used by a compute shader
        r->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(volumeTexture.texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        hasViewChanged = true;
    }
}



BRWL_RENDERER_NS_END
