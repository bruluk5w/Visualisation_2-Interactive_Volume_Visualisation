#include "TextureManager.h"

#include "BaseTextureHandle.h"
#include "BaseTexture.h"

BRWL_RENDERER_NS


BaseTextureManager::BaseTextureManager() : registry{ { }, { }, { } }
{
    static_assert(std::is_base_of_v<BaseTextureManager, TextureManager>, "The platform texture manager has to be derived from BaseTextureManager.");
}

BaseTextureManager::~BaseTextureManager()
{
    destroyAll();
}

void BaseTextureManager::destroy(BaseTextureHandle& handle)
{
    BaseTexture* tex = remove(handle.id);
    delete tex;
    handle = BaseTextureHandle::Invalid;
}

BaseTexture* BaseTextureManager::get(const BaseTextureHandle& handle, id_type* idx/*=nullptr*/) const
{
    checkHandle(handle);
    std::scoped_lock l(registry.registryLock);
    id_type index = getIndex(handle.id);
    if (idx) *idx = index;
    BaseTexture* tex = registry.store[index];
    BRWL_CHECK(tex != nullptr, nullptr);
    return tex;
}

void BaseTextureManager::destroyAll()
{
    std::scoped_lock l(registry.registryLock);
    for (BaseTexture* tex : registry.store)
    {
        if (tex != nullptr)
        {
            delete tex;
        }
    }

    registry.store.clear();
    registry.index.clear();
}

BaseTextureManager::id_type BaseTextureManager::add(BaseTexture* tex)
{
    std::scoped_lock l(registry.registryLock);

    BRWL_CHECK(std::find(registry.store.begin(), registry.store.end(), tex) == registry.store.end(), BRWL_CHAR_LITERAL("Cannot add the same texture twice."));

    const auto idx = std::find(registry.index.begin(), registry.index.end(), BaseTextureHandle::Invalid.id);

    if (idx == registry.index.end())
    {
        const id_type new_idx = registry.index.size();
        registry.index.push_back(new_idx);
        registry.store.push_back(tex);
        return new_idx;
    }

    const auto slot = std::find(registry.store.begin(), registry.store.end(), nullptr);
    BRWL_CHECK(slot != registry.store.end(), nullptr);
    *slot = tex;
    *idx = slot - registry.store.begin();

    return *idx;
}

void BaseTextureManager::remove(BaseTexture* tex)
{
    std::scoped_lock l(registry.registryLock);
    const auto slot = std::find(registry.store.begin(), registry.store.end(), tex);

    BRWL_CHECK(slot != registry.store.end(), BRWL_CHAR_LITERAL("Cannot remove texture because it doesn't exist in the registry."));

    const id_type index = slot - registry.store.begin();

    const auto idx = std::find(registry.index.begin(), registry.index.end(), index);
    BRWL_CHECK(idx != registry.index.end(), nullptr);
 
    *slot = nullptr;
    *idx = BaseTextureHandle::Invalid.id;
}

BaseTexture* BaseTextureManager::remove(id_type id)
{
    checkTextureId(id);
    std::scoped_lock l(registry.registryLock);
    const id_type idx = getIndex(id);
    registry.index[idx] = BaseTextureHandle::Invalid.id;
    BaseTexture* tex = registry.store[idx];
    BRWL_CHECK(tex != nullptr, nullptr);
    registry.store[idx] = nullptr;
    
    return tex;
}

void BaseTextureManager::checkHandle(const BaseTextureHandle& handle) const
{
    BRWL_EXCEPTION(handle.mgr == this, BRWL_CHAR_LITERAL("Texture manager method  called with a texture which this manager does not own."));
    checkTextureId(handle.id);
}

void BaseTextureManager::checkTextureId(const BaseTextureManager::id_type id)
{
    BRWL_EXCEPTION(id >= 0 && id != BaseTextureHandle::Invalid.id, BRWL_CHAR_LITERAL("Invalid texture ID."));
}

BaseTextureManager::id_type BaseTextureManager::getIndex(const id_type id) const
{
    BRWL_EXCEPTION(id < registry.index.size(), BRWL_CHAR_LITERAL("Texture ID out of bounds."));
    const id_type idx = registry.index[id];
    BRWL_CHECK(idx >= 0 && idx < registry.store.size(), nullptr);
    return idx;
}


BRWL_RENDERER_NS_END
