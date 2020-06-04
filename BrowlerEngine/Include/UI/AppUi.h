#pragma once // (c) 2020 Lukas Brunner

BRWL_NS

struct UIResult
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

};

void renderAppUI(UIResult& result, const UIResult& values);

BRWL_NS_END