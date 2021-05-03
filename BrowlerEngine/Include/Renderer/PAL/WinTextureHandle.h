#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

#include "BaseTextureHandle.h"

#include "WinTextureManager.h"

BRWL_RENDERER_NS


namespace PAL
{


	struct WinTextureHandle : public BaseTextureHandle
	{
		friend class PAL::WinTextureManager;
#define HANDLE_TYPE_ WinTextureHandle
		FOR_EACH_WIN_TEXTURE_HANDLE_METHOD(TEXTURE_HANDLE_METHOD_DECL)
#undef HANDLE_TYPE_
		
	};


}


BRWL_RENDERER_NS_END


#endif // BRWL_PLATFORM_WINDOWS
