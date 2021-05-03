#pragma once // (c) 2020 Lukas Brunner

#ifdef BRWL_PLATFORM_WINDOWS

BRWL_RENDERER_NS

namespace PAL
{


	class DescriptorHandle;

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

		void destroy();

		DescriptorHandle* descriptorHandle = nullptr;
		ComPtr<ID3D12Resource> texture = nullptr;
		ComPtr<ID3D12Resource> uploadHeap = nullptr;
		State state = State::NONE;
	};


} // namespace PAL

BRWL_RENDERER_NS_END

#endif
