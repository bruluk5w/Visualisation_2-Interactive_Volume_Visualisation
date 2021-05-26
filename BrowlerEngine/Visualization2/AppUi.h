#pragma once // (c) 2020 Lukas Brunner

#include "UI/ImGui/ImGuiHelpers.h"

#include "Renderer/Texture.h"

BRWL_NS


enum class TransferFunctionBitDepth : uint8_t
{
	BIT_DEPTH_8_BIT = 0,
	BIT_DEPTH_10_BIT,
	MAX,
	MIN = 0
};

template<RENDERER::SampleFormat S, template<RENDERER::SampleFormat> typename T = RENDERER::Texture>
struct TransferFunction
{
	TransferFunction() :
		bitDepth(TransferFunctionBitDepth::BIT_DEPTH_8_BIT),
		controlPoints(),
		transferFunction{ 0 },
		textureID(nullptr)
	{
		updateFunction();
	}

	using sampleT = typename T<S>::sampleT;
	TransferFunctionBitDepth bitDepth;

	::ImGui::CtrlPointGroup controlPoints[T<S>::sampleByteSize()];
	sampleT transferFunction[1 << 10];
	void* textureID;

	TransferFunction<S>& operator=(const TransferFunction<S>& o)
	{
		memcpy(transferFunction, o.transferFunction, sizeof(transferFunction));
		bitDepth = o.bitDepth;
		controlPoints = o.controlPoints;
		return *this;
	}

	TransferFunction<S> operator==(const TransferFunction<S> o) {
		return bitDepth == o.bitDepth &&
			memcmp(transferFunction, o.transferFunction, getArrayLength() * sizeof(UIResult::TransferFunction<S>::sampleT)) != 0;
	}

	TransferFunction<S> operator=(const TransferFunction<S> o) { return operator==(o); }


	int getArrayLength() const
	{
		switch (bitDepth) {
		case TransferFunctionBitDepth::BIT_DEPTH_8_BIT:
			return 1 << 8;
		case TransferFunctionBitDepth::BIT_DEPTH_10_BIT:
			return 1 << 10;
		default:
			BRWL_UNREACHABLE();
		}

		return 0;
	}

	void updateFunction()
	{
		const int numPoints = (int)controlPoints.points.size();

		thread_local std::vector<Vec2> vBuf;
		vBuf.reserve(numPoints);

		for (const size_t ref : controlPoints.refs)
			vBuf.push_back(controlPoints.points[ref].pt);

		if (!BRWL_VERIFY(numPoints >= 2, nullptr)) return;

		// evaluate the spline at non-uniform locations
		if (!BRWL_VERIFY(vBuf[0].x == 0 && vBuf[numPoints - 1].x == 1, nullptr)) {
			return;
		}
		Vec2 first(vBuf[0] - (vBuf[1] - vBuf[0]));
		Vec2 last(vBuf[numPoints - 1] + (vBuf[numPoints - 1] - vBuf[numPoints - 2]));
		const int numSamples = getArrayLength();
		const int numSubdivisions = 30;
		const int numTessellatedPoints = (numPoints - 1) * numSubdivisions + 1;
		std::unique_ptr<Vec2[]> tessellatedPoints = std::unique_ptr<Vec2[]>(new Vec2[numTessellatedPoints]);
		Splines::CentripetalCatmullRom(first, vBuf.data(), last, vBuf.size(), tessellatedPoints.get(), numSubdivisions, numTessellatedPoints);

		// Interpolate along segments for approximation of uniform intervals along the x axis
		int cursor = 1;
		transferFunction[0] = tessellatedPoints[0].y;
		for (int i = 1; i < numSamples; ++i)
		{
			while (tessellatedPoints[cursor].x * numSamples <= i && cursor < numTessellatedPoints - 1)
			{
				++cursor;
			}
			const Vec2& previous = tessellatedPoints[cursor - 1];
			const Vec2& next = tessellatedPoints[cursor];
			const Vec2 delta = next - previous;

			transferFunction[i] = previous.y + ((float)i / (float)numSamples - previous.x) * delta.y / delta.x;
		}

		transferFunction[numSamples - 1] = tessellatedPoints[numTessellatedPoints - 1].y;

		vBuf.clear();
	}
};

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

		TransferFunction<RENDERER::SampleFormat::F32> refractionTansFunc;
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

void renderAppUI(UIResult& result, const UIResult& values);

BRWL_NS_END