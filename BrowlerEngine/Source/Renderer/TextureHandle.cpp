#include "TextureHandle.h"


BRWL_RENDERER_NS


static_assert(std::is_base_of_v<BaseTextureHandle, TextureHandle> && sizeof(BaseTextureHandle) == sizeof(TextureHandle));


BRWL_RENDERER_NS_END
