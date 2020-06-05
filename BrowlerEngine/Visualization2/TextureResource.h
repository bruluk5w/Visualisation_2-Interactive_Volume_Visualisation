#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

struct TextureResource
{
	enum class State : uint8_t
	{
		UNKNOWN = 0,
		LOADING,
		RESIDENT,
		FAILED,
		MAX,
		MIN = 0
	};

	PAL::DescriptorHeap::Handle descriptorHandle = { 0, 0 };
	ComPtr<ID3D12Resource> texture = nullptr;
	State state = State::UNKNOWN;

	void destroy()
	{
		descriptorHandle = { 0, 0 };
		texture = nullptr;
		state = State::UNKNOWN;
	}
};

BRWL_RENDERER_NS_END