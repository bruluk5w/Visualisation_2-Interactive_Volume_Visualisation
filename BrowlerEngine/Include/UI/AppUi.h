#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

struct UIResult
{
	struct Settings
	{
		enum class Font : uint8_t
		{
			OPEN_SANS_LIGHT = 0,
			OPEN_SANS_REGULAR,
			OPEN_SANS_SEMIBOLD,
			OPEN_SANS_BOLD,
			MAX,
			MIN = 0
		} font;

		float fontSize;

		static const char* fontNames[ENUM_CLASS_TO_NUM(Font::MAX)];
	} settings;

	struct TransferFunction
	{
		using sampleT = float;
		enum class BitDepth {
			BIT_DEPTH_8_BIT = 0,
			BIT_DEPTH_10_BIT,
			MAX,
			MIN = 0
		} bitDepth;

		static const char* bitDepthNames[ENUM_CLASS_TO_NUM(BitDepth::MAX)];
		int getArrayLenth();
		sampleT transferFunction[1<<10];
		void* textureID;
	} transferFunction;

};

void renderAppUI(UIResult& result, const UIResult& values);

BRWL_NS_END