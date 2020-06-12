#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

struct TextureResource
{
	enum class State : uint8_t
	{
		UNKNOWN = 0,
		REQUESTING_UPLOAD,
		LOADING,
		RESIDENT,
		FAILED,
		MAX,
		MIN = 0
	};

	PAL::DescriptorHeap::Handle descriptorHandle = { };
	ComPtr<ID3D12Resource> texture = nullptr;
	ComPtr<ID3D12Resource> uploadHeap = nullptr;
	State state = State::UNKNOWN;

	void destroy()
	{
		descriptorHandle.destroy();
		descriptorHandle = { };
		texture = nullptr;
		uploadHeap = nullptr;
		state = State::UNKNOWN;
	}
};

BRWL_RENDERER_NS_END