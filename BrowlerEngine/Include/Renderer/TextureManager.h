#pragma once // (c) 2020 Lukas Brunner

#include <vector>

BRWL_RENDERER_NS

class BaseTexture;

class TextureManager {
public:
	TextureManager(ID3D12Device* device) :
		device(device),
		uploadCommandQueue(nullptr),
		uploadCommandAllocator(nullptr),
		uploadCommandList(nullptr)
	{ }

	ID3D12Device* getDevice() const { return device; }

private:
	friend class BaseTexture;

	void add(BaseTexture* tex);
	void remove(BaseTexture* tex);
	//! Updates the textures managed by teh TextureManager
	/**
	 * Updates the textures which request upload and swaps the staging textures with the live textures in case they are ready.
	 * 
	 * \return True if the the TextureManager touched any of the Textures, potentially causeing descriptors to be modified. In this case the respective descriptor heap needs an update.
	 */
	bool update();

	std::mutex registryAccess;
	std::vector<BaseTexture*> registry;

	ID3D12Device* device;
	ComPtr<ID3D12CommandQueue> uploadCommandQueue;
	ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
};


BRWL_RENDERER_NS_END
