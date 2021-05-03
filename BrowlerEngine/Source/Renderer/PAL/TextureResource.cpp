#include "PAL/TextureResource.h"

#ifdef BRWL_PLATFORM_WINDOWS

#include "PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

namespace PAL
{


	void TextureResource::destroy()
	{
		if (descriptorHandle) descriptorHandle->release();
		descriptorHandle = nullptr;
		texture = nullptr;
		uploadHeap = nullptr;
		state = State::NONE;
	}


} // namespace PAL

BRWL_RENDERER_NS_END

#endif
