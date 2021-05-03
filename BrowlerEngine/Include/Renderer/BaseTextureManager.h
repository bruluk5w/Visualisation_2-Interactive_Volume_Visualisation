#pragma once // (c) 2020 Lukas Brunner

#include "TextureManagerFwd.h"

BRWL_RENDERER_NS


class BaseTexture;


// All texture manager methods available through the base texture handle are listed here (return type, name, (arguments), const)
#define FOR_EACH_BASE_TEXTURE_HANDLE_METHOD(f) \
	f(void, destroy, ()) \
	f(bool, startLoad, ()) \
	f(bool, isResident, (), const) \


struct BaseTextureHandle;

class BaseTextureManager
{
	friend class BaseTexture;
	friend struct BaseTextureHandle;

public:
	using id_type = int16_t;
	BaseTextureManager();

	virtual ~BaseTextureManager();

	//! Return a handle to a new texture
	template<typename T>
	BaseTextureHandle createTexture(const BRWL_CHAR* name) {
		static_assert(std::is_base_of_v<BaseTexture, T>, "Texture type must be a subtype of BaseTexture.");
		BRWL_EXCEPTION(registry.index.size() < std::numeric_limits<id_type>::max(), BRWL_CHAR_LITERAL("Too many textures. Increase id_type."));
		BaseTexture* t = new T(name);
		return { this, add(t) };
	};

	//! Get the underlying CPU texture. 
	BaseTexture* get(const BaseTextureHandle& handle, id_type* idx=nullptr) const;

	//! Remove and destroy all textures managed by the texture manager
	virtual void destroyAll();

#pragma region Texture handle methods
	 
	//! Remove and destroy the texture of this handle
	virtual void destroy(BaseTextureHandle& handle);

	//! Dispatch loading the texture
	/**
	 * Implicitly initializes GPU resources if they don't exist yet.
	 **/
	virtual bool startLoad(const BaseTextureHandle& handle) = 0;
	
	//! Returns true if the GPU resources are resident in GPU memory
	virtual bool isResident(const BaseTextureHandle& handle) const = 0;

#pragma endregion Texture handle methods

	//! Updates the textures managed by the TextureManager
	/**
	 * Dispatches updating the textures on the GPU.
	 * return True if any GPU resources were touched.
	 */
	virtual bool update() = 0;

	//! Promotes staged textures to live textures
	/**
	 * Swaps pointers to staged textures with pointers to live textures if staged textures are resident.
	 * return True if any staged texture became resident and has been promoted to live.
	 */
	virtual bool promoteStagedTextures() = 0;

	//! Blocks until a staged texture becomes available to the CPU.
	virtual void waitForPendingUploads(BaseTextureHandle* handles, id_type numHandles) = 0;

protected:

	id_type add(BaseTexture* tex);
	void remove(BaseTexture* tex);
	BaseTexture* remove(id_type id);
	
	void checkHandle(const BaseTextureHandle& handle) const;
	static void checkTextureId(const BaseTextureManager::id_type id);
	id_type getIndex(const id_type id) const;

	struct
	{
		std::vector<BaseTexture*> store;
		std::vector<id_type> index;
		// todo: rename because it's actually a lock for the whole texture manager
		mutable std::recursive_mutex registryLock;  // todo: replace with normal mutex and unguarded private impl methods
	} registry;

	TextureManager* derivedThis;
};


BRWL_RENDERER_NS_END
