#include "BaseTextureHandle.h"

#include "TextureHandle.h"
#include "BaseTextureManager.h"

BRWL_RENDERER_NS


const BaseTextureHandle BaseTextureHandle::Invalid(nullptr, -1);

#define HANDLE_TYPE_ BaseTextureHandle
FOR_EACH_BASE_TEXTURE_HANDLE_METHOD(TEXTURE_HANDLE_METHOD_IMPL)
#undef HANDLE_TYPE_


TextureHandle* BaseTextureHandle::asPlatformHandle()
{
    // This is undefined behaviour but works in this trivial case since the derived class only adds its independent methods
    return reinterpret_cast<TextureHandle*>(this);
}


const TextureHandle* BaseTextureHandle::asPlatformHandle() const
{
    // This is undefined behaviour but works in this trivial case since the derived class only adds its independent methods
    return reinterpret_cast<const TextureHandle*>(this);
}


BRWL_RENDERER_NS_END
