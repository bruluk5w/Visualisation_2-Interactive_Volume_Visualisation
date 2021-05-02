#pragma once

#include "BaseTextureManager.h"

BRWL_RENDERER_NS


namespace PAL {
#ifdef BRWL_PLATFORM_WINDOWS
	class WinTextureManager;
#endif
}

#define BRWL_CHECK_NULL_MGR_ BRWL_EXCEPTION(mgr, nullptr)

#define HANDLE_IMPL(ret_type, f_name, args, ...) \
	ret_type f_name(BRWL_STRIP_PARENTHESIS(args)) BRWL_FIRST_ARG(__VA_ARGS__) { \
		BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); \
		return mgr->f_name(*this BRWL_APPEND_ARGS(args)); \
	}

struct BaseTextureHandle
{
	friend class BaseTextureManager;
#ifdef BRWL_PLATFORM_WINDOWS
	friend class PAL::WinTextureManager;
#endif

	FOR_EACH_TEXTURE_HANDLE_METHOD(HANDLE_IMPL)

		static const BaseTextureHandle Invalid;

	BaseTextureHandle(BaseTextureManager* mgr, BaseTextureManager::id_type id) : mgr(mgr), id(id) { }

	bool operator==(const BaseTextureHandle& other) const { return memcmp(this, &other, sizeof(BaseTextureHandle)); }
	bool operator!=(const BaseTextureHandle& other) const { return !(*this == other); }

	BaseTexture* operator->() { BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); return mgr->get(*this); }
	const BaseTexture* operator->() const { BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); return mgr->get(*this); }
	BaseTexture& operator*() { BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); return *mgr->get(*this); }
	const BaseTexture& operator*() const { BRWL_IF_DEBUG(BRWL_CHECK_NULL_MGR); return *mgr->get(*this); }

private:
	BaseTextureManager* mgr;
	BaseTextureManager::id_type id;
};

#undef HANDL_IMPL
#undef BRWL_CHECK_NULL_MGR_


BRWL_RENDERER_NS_END
