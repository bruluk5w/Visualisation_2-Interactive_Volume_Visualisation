#pragma once // (c) 2020 Lukas Brunner

#include <vector>

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

		float voxelsPerCm;
		float numSlicesPerVoxel;
		bool vsync;
		bool freeCamMovement;
		bool drawAssetBoundaries;
		bool drawViewingVolumeBoundaries;
		bool drawOrthographicXRay;
	} settings;

	struct TransferFunction
	{
		TransferFunction();
		using sampleT = float;
		enum class BitDepth {
			BIT_DEPTH_8_BIT = 0,
			BIT_DEPTH_10_BIT,
			MAX,
			MIN = 0
		} bitDepth;

		static const char* bitDepthNames[ENUM_CLASS_TO_NUM(BitDepth::MAX)];


		std::vector<Vec2> controlPoints;
		sampleT transferFunction[1<<10];
		void* textureID;
		
		int getArrayLength() const;
		void updateFunction();
	};

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
	union TransferFunctionCollection
	{
		TransferFunctionCollection();
		~TransferFunctionCollection();
		// All our transferfunctions
		TransferFunction array[4];

		// aliases
		struct Aliases {
			TransferFunction refractionTansFunc;
			TransferFunction particleColorTransFunc;
			TransferFunction opacityTransFunc;
			TransferFunction mediumColorTransFunc;
		} functions;

		static_assert(sizeof(functions) == sizeof(array));

		static const char* transferFuncNames[ENUM_CLASS_TO_NUM(TransferFuncType::MAX)];

	} transferFunctions;

	struct Light
	{
		Vec3 coords;
		Vec4 color;
	} light;
};

void renderAppUI(UIResult& result, const UIResult& values);

BRWL_NS_END