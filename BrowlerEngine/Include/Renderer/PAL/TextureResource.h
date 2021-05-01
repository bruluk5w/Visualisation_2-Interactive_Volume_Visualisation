#pragma once // (c) 2020 Lukas Brunner

#include "PAL/DescriptorHeap.h"


BRWL_RENDERER_NS

// A basic texture meant to be uploaded and sampled from
struct TextureResource
{
	enum class State : uint8_t
	{
		NONE = 0,
		REQUESTING_UPLOAD,
		LOADING,
		RESIDENT,
		FAILED,
		MAX,
		MIN = 0
	};

	bool isResident() const { return state == State::RESIDENT; }

	void destroy()
	{
		if (descriptorHandle) descriptorHandle->release();
		descriptorHandle = nullptr;
		texture = nullptr;
		uploadHeap = nullptr;
		state = State::NONE;
	}

	PAL::DescriptorHandle* descriptorHandle = nullptr;
	ComPtr<ID3D12Resource> texture = nullptr;
	ComPtr<ID3D12Resource> uploadHeap = nullptr;
	State state = State::NONE;

};

 BRWL_RENDERER_NS_END