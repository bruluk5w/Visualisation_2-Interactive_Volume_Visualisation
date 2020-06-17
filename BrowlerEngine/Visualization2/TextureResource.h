#pragma once // (c) 2020 Lukas Brunner

#include "Common/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

// A basic texture meant to be uploaded and sampled from
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

	PAL::DescriptorHandle* descriptorHandle = nullptr;
	ComPtr<ID3D12Resource> texture = nullptr;
	ComPtr<ID3D12Resource> uploadHeap = nullptr;
	State state = State::UNKNOWN;

	void destroy()
	{
		if (descriptorHandle) descriptorHandle->release();
		descriptorHandle = nullptr;
		texture = nullptr;
		uploadHeap = nullptr;
		state = State::UNKNOWN;
	}
};

BRWL_RENDERER_NS_END