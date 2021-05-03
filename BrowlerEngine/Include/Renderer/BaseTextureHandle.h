#pragma once // (c) 2020 Lukas Brunner

#include "BaseTextureManager.h"

#include "TextureHandleFwd.h"

BRWL_RENDERER_NS


#define BRWL_CHECK_NULLPTR_(mgr) BRWL_EXCEPTION(mgr, nullptr)

#define TEXTURE_HANDLE_METHOD_DECL(ret_type, f_name, args, ...) \
	ret_type f_name(BRWL_STRIP_PARENTHESIS(args)) BRWL_FIRST_ARG(__VA_ARGS__);


#define TEXTURE_HANDLE_METHOD_IMPL(ret_type, f_name, args, ...) \
	ret_type HANDLE_TYPE_::f_name(BRWL_STRIP_PARENTHESIS(args)) BRWL_FIRST_ARG(__VA_ARGS__) { \
		BRWL_IF_DEBUG(BRWL_CHECK_NULLPTR, mgr); \
		return platformMgr()->f_name(*this BRWL_APPEND_ARGS(args)); \
	}

struct BaseTextureHandle
{
	friend class BaseTextureManager;
#ifdef BRWL_PLATFORM_WINDOWS
	friend class PAL::WinTextureManager;  // cannot declare typedef as friend until C++ 20
#endif

	FOR_EACH_BASE_TEXTURE_HANDLE_METHOD(TEXTURE_HANDLE_METHOD_DECL)

	static const BaseTextureHandle Invalid;

	BaseTextureHandle(BaseTextureManager* mgr, BaseTextureManager::id_type id) : mgr(mgr), id(id) { }

	bool operator==(const BaseTextureHandle& other) const { return memcmp(this, &other, sizeof(BaseTextureHandle)); }
	bool operator!=(const BaseTextureHandle& other) const { return !(*this == other); }

	BaseTexture* operator->() { BRWL_IF_DEBUG(BRWL_CHECK_NULLPTR, mgr); return mgr->get(*this); }
	const BaseTexture* operator->() const { BRWL_IF_DEBUG(BRWL_CHECK_NULLPTR, mgr); return mgr->get(*this); }
	BaseTexture& operator*() { BRWL_IF_DEBUG(BRWL_CHECK_NULLPTR, mgr); return *mgr->get(*this); }
	const BaseTexture& operator*() const { BRWL_IF_DEBUG(BRWL_CHECK_NULLPTR, mgr); return *mgr->get(*this); }

	TextureHandle* asPlatformHandle();
	const TextureHandle* asPlatformHandle() const;

protected:
	TextureManager* platformMgr() const { return mgr->derivedThis; }

	BaseTextureManager* mgr;
	BaseTextureManager::id_type id;
};


BRWL_RENDERER_NS_END
