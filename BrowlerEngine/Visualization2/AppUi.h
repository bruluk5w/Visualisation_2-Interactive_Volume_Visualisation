#pragma once // (c) 2020 Lukas Brunner

#include "UI/ImGui/ImGuiHelpers.h"
#include "GuiTransferFunction.h"

BRWL_NS


struct UIResult
{
	UIResult();

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

		float numSlicesPerVoxel;
		bool vsync;
		bool freeCamMovement;
		bool drawAssetBoundaries;
		bool drawViewingVolumeBoundaries;
		bool drawOrthographicXRay;
	} settings;

	enum class TransferFuncType : uint8_t
	{
		REFRACTION = 0,
		PARTICLE_COLOR,
		OPACITY,
		MEDIUM_COLOR,
		MAX,
		MIN = 0
	};

	// A collection of transfer functions
	struct TransferFunctionCollection
	{
		TransferFunctionCollection();

		BaseTransferFunction* array[ENUM_CLASS_TO_NUM(TransferFuncType::MAX)];

		TransferFunction<RENDERER::SampleFormat::F32> refractionTransFunc;
		TransferFunction<RENDERER::SampleFormat::VEC4F32> particleColorTransFunc;
		TransferFunction<RENDERER::SampleFormat::F32> opacityTransFunc;
		TransferFunction<RENDERER::SampleFormat::VEC4F32> mediumColorTransFunc;


		static const char* transferFuncNames[ENUM_CLASS_TO_NUM(TransferFuncType::MAX)];

	} transferFunctions;

	struct Light
	{
		Vec3 coords;
		Vec4 color;
		bool operator==(const Light& o) { return coords == o.coords && color == o.color; }
		bool operator!=(const Light& o) { return !(*this == o); }
	} light;

	unsigned int remainingSlices;
};

void setupAppUI();

void renderAppUI(UIResult& result, const UIResult& values);

BRWL_NS_END