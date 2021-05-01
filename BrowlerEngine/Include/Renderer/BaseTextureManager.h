#pragma once // (c) 2020 Lukas Brunner

#include <functional>

BRWL_RENDERER_NS


class BaseTexture;

namespace PAL {
#ifdef BRWL_PLATFORM_WINDOWS
	class WinTextureManager;
#endif
}


// All texture manager methods available through the texture handle are listed here (return type, name, (arguments), const)
#define FOR_EACH_TEXTURE_HANDLE_METHOD(f) \
	f(void, destroy, ()) \
	f(virtual bool, load, ()) \
	f(virtual bool, isResident, (), const) \

	//f(void, create, (uint16_t sizeX, uint16_t sizeY, uint16_t sizeZ = 1)) \
	//f(void, clear, ()) \
	//f(uint16_t, getSizeX, (), const) \
	//f(uint16_t, getSizeY, (), const) \
	//f(uint16_t, getSizeZ, (), const) \
	//f(uint32_t, getStrideX, (), const) \
	//f(uint32_t, getStrideY, (), const) \
	//f(uint32_t, getStrideZ, (), const) \
	//f(uint32_t, getStrideZ, (), const) \
	//f(uint32_t, getBufferSize, (), const) \
	//f(const BRWL_CHAR*, getName, (), const)



struct TextureHandle;

class BaseTextureManager
{
	friend class BaseTexture;
	friend struct TextureHandle;

protected:
	using id_type = int16_t;

public:
	BaseTextureManager() : registry{ { }, { }, { } }
	{ }

	virtual ~BaseTextureManager();

	//! Return a handle to a new texture
	template<typename T>
	TextureHandle createTexture(const BRWL_CHAR* name) {
		static_assert(std::is_base_of_v<BaseTexture, T>, "Texture type must be a subtype of BaseTexture.");
		BRWL_EXCEPTION(registry.index.size() < std::numeric_limits<id_type>::max(), BRWL_CHAR_LITERAL("Too many textures. Increase id_type."));
		BaseTexture* t = new T(name);
		return { this, add(t) };
	};

	//! Get the underlying CPU texture. 
	BaseTexture* get(const TextureHandle& handle, id_type* idx=nullptr) const;
	//! Remove and destroy all textures managed by the texture manager
	void destroyAll();

	//! Remove and destroy the texture of this handle
	void destroy(TextureHandle& handle);
	//! Creates GPU resources for this texture
	virtual bool load(const TextureHandle& handle) = 0;
	//! Returns true if the GPU resources are resident in GPU memory
	virtual bool isResident(const TextureHandle& handle) const = 0;

	//! Updates the textures managed by the TextureManager
	/**
	 * Dispatches updating the textures on the GPU.
	 * return True if any GPU resources were touched.
	 */
	virtual bool update() = 0;

	//! Blocks until all resources are synchronized with the GPU.
	//virtual void waitForPendingUploads() = 0;

protected:

	id_type add(BaseTexture* tex);
	void remove(BaseTexture* tex);
	BaseTexture* remove(id_type id);
	
	static void checkTextureId(const BaseTextureManager::id_type id);
	id_type getIndex(const id_type id) const;

	struct
	{
		std::vector<BaseTexture*> store;
		std::vector<id_type> index;
		mutable std::recursive_mutex registryLock;  // todo: replace with normal mutex and unguarded private impl methods
	} registry;
};

#define BRWL_CHECK_NULL_MGR_ BRWL_EXCEPTION(mgr, nullptr)

#define HANDLE_IMPL(ret_type, f_name, args, ...) \
	ret_type f_name(BRWL_STRIP_PARENTHESIS(args)) BRWL_FIRST_ARG(__VA_ARGS__) { \
		BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); \
		return mgr->f_name(*this BRWL_APPEND_ARGS(args)); \
	}

struct TextureHandle
{
	friend class BaseTextureManager;
#ifdef BRWL_PLATFORM_WINDOWS
	friend class PAL::WinTextureManager;
#endif

	FOR_EACH_TEXTURE_HANDLE_METHOD(HANDLE_IMPL)
	
	static const TextureHandle Invalid;

	TextureHandle(BaseTextureManager* mgr, BaseTextureManager::id_type id) : mgr(mgr), id(id) { }

	bool operator==(const TextureHandle& other) const { return memcmp(this, &other, sizeof(TextureHandle)); }
	bool operator!=(const TextureHandle& other) const { return !(*this == other); }

	BaseTexture* operator->() { BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); return mgr->get(*this); }

private:
	BaseTextureManager* mgr;
	BaseTextureManager::id_type id;
};

#undef HANDL_IMPL
#undef BRWL_CHECK_NULL_MGR_




BRWL_RENDERER_NS_END
