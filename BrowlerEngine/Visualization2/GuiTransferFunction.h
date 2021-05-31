#pragma once

#include "Renderer/Texture.h"
#include "UI/ImGui/ImGuiHelpers.h"
#include "Common/Spline.h"

#include <array>


BRWL_NS


template<RENDERER::SampleFormat S, template<RENDERER::SampleFormat> typename T = RENDERER::Texture> 
struct TransferFunction;

struct BaseTransferFunction
{
	template<RENDERER::SampleFormat U, template<RENDERER::SampleFormat> typename V> friend struct TransferFunction; // access protected methods from sibling templates

	BaseTransferFunction();

	unsigned int getArrayLength() const;
	virtual uint8_t getNumChannels() const = 0;
	virtual uint8_t getChannelOffsets(uint8_t channelIdx) const = 0;
	virtual uint8_t getChannelStrides(uint8_t channelIdx) const = 0;
	virtual ::ImGui::CtrlPointGroup* getCtrlPointGroup(uint8_t channelIdx) = 0;
	virtual const float* getData() const = 0;
	virtual float* getData() = 0;

	virtual void updateFunction(const bool* dirtyFlag) = 0;

	bool operator==(const BaseTransferFunction& o) { return equals(o); }
	bool operator!=(const BaseTransferFunction& o) {
		return !operator==(o);
	}

protected:
	// double dispatch via visitor pattern for equality and assignment operations
	virtual bool equals(const BaseTransferFunction& o) const;
	virtual bool equals(const TransferFunction<RENDERER::SampleFormat::F32>& o) const ;
	virtual bool equals(const TransferFunction<RENDERER::SampleFormat::VEC4F32>& o) const;

	virtual BaseTransferFunction& copy(const BaseTransferFunction& o);
	virtual void copyTo(TransferFunction<RENDERER::SampleFormat::F32>& o) const;
	virtual void copyTo(TransferFunction<RENDERER::SampleFormat::VEC4F32>& o) const;

public:
	BaseTransferFunction& operator=(const BaseTransferFunction& o) { return copy(o); }

	enum class BitDepth : uint8_t
	{
		BIT_DEPTH_8_BIT = 0,
		BIT_DEPTH_10_BIT,
		MAX,
		MIN = 0
	} bitDepth;

	static const char* bitDepthNames[ENUM_CLASS_TO_NUM(BitDepth::MAX)];

	void* textureID;
};

//S is expected to be a format with only 32bit float channels
template<RENDERER::SampleFormat S, template<RENDERER::SampleFormat> typename T>
struct TransferFunction : public BaseTransferFunction
{
	
	template<RENDERER::SampleFormat U, template<RENDERER::SampleFormat> typename V> friend struct TransferFunction; // access protected methods from sibling templates

	static constexpr uint8_t numChannels()
	{
		if constexpr (S == RENDERER::SampleFormat::VEC4F32)
			return 3;
		else
			return 1;
	}

	using sampleT = typename T<S>::sampleT;

	::ImGui::CtrlPointGroup controlPoints[numChannels()];
	sampleT transferFunction[1 << 10];


	TransferFunction() : BaseTransferFunction(),
		controlPoints(),
		transferFunction {}
	{
		constexpr std::array<bool, numChannels()> dirtyFlags{ []() {
			std::array<bool, numChannels()> res = {0};
			for (int i = 0; i < numChannels(); ++i) { res[i] = true; }
			return res;
		}() };

		updateFunction(dirtyFlags.data()); // initializes transferFunction

		// set unused channels to one (e.g. alpha)
		for (int i = numChannels(); i < sizeof(sampleT) / sizeof(float); ++i)
		{
			float* base = getData() + i;
			for (unsigned int j = 0; j < countof(transferFunction); ++j) {
				base[j * sizeof(sampleT) / sizeof(float)] = 1.0f;
			}
		}
	}


	virtual uint8_t getNumChannels() const override
	{
		return numChannels();
	}

	virtual uint8_t getChannelOffsets(uint8_t channelIdx) const override
	{
		BRWL_CHECK(channelIdx < getNumChannels(), nullptr);
		if constexpr(S == RENDERER::SampleFormat::VEC4F32)
			return channelIdx;
		else
			return 1;
	}

	virtual uint8_t getChannelStrides(uint8_t channelIdx) const override
	{
		BRWL_CHECK(channelIdx < getNumChannels(), nullptr);
		if constexpr (S == RENDERER::SampleFormat::VEC4F32)
			return 4;
		else
			return 1;
	}

	virtual ::ImGui::CtrlPointGroup* getCtrlPointGroup(uint8_t channelIdx) override
	{
		BRWL_CHECK(channelIdx < getNumChannels(), nullptr);
		return controlPoints + channelIdx;
	}


	virtual const float* getData() const override
	{
		if constexpr (S == RENDERER::SampleFormat::VEC4F32)
			return &transferFunction[0].x;
		else
			return transferFunction;
	}

	virtual float* getData() override
	{
		if constexpr (S == RENDERER::SampleFormat::VEC4F32)
			return &transferFunction[0].x;
		else
			return transferFunction;
	}

	// dirtyFlag indicates which functions should be updated, must pass as many bools as there are channels
	virtual void updateFunction(const bool* dirtyFlag) override
	{
		thread_local std::vector<Vec2> vBuf;

		for (int i = 0; i < numChannels(); ++i)
		{
			if (!dirtyFlag[i]) continue;

			::ImGui::CtrlPointGroup& group = controlPoints[i];

			const unsigned int numPoints = (int)group.points.size();

			vBuf.reserve(numPoints);

			for (const size_t ref : group.refs)
				vBuf.push_back(group.points[ref].pt);

			if (!BRWL_VERIFY(numPoints >= 2, nullptr)) return;

			// evaluate the spline at non-uniform locations
			if (!BRWL_VERIFY(vBuf[0].x == 0 && vBuf[numPoints - 1].x == 1, nullptr)) {
				return;
			}

			Vec2 first(vBuf[0] - (vBuf[1] - vBuf[0]));
			Vec2 last(vBuf[numPoints - 1] + (vBuf[numPoints - 1] - vBuf[numPoints - 2]));
			const unsigned int numSamples = getArrayLength();
			const unsigned int numSubdivisions = 30;
			const unsigned int numTessellatedPoints = (numPoints - 1) * numSubdivisions + 1;
			std::unique_ptr<Vec2[]> tessellatedPoints = std::unique_ptr<Vec2[]>(new Vec2[numTessellatedPoints]);
			::BRWL::Splines::CentripetalCatmullRom(first, vBuf.data(), last, vBuf.size(), tessellatedPoints.get(), numSubdivisions, numTessellatedPoints);

			// Interpolate along segments for approximation of uniform intervals along the x axis
			const uint8_t stride = getChannelStrides(i);
			float* data = getData() + getChannelOffsets(i);
			unsigned int cursor = 1;
			data[0] = tessellatedPoints[0].y;
			for (unsigned int i = 1; i < numSamples; ++i)
			{
				while (tessellatedPoints[cursor].x * numSamples <= i && cursor < numTessellatedPoints - 1)
				{
					++cursor;
				}

				const Vec2& previous = tessellatedPoints[cursor - 1];
				const Vec2& next = tessellatedPoints[cursor];
				const Vec2 delta = next - previous;

				data[stride * i] = Utils::max((previous.y + ((float)i / (float)numSamples - previous.x) * delta.y / delta.x), 0.f);
			}

			data[stride * (numSamples - 1)] = Utils::max(tessellatedPoints[numTessellatedPoints - 1].y, 0.f);
			vBuf.clear();
		}
	}

	protected:
		virtual bool equals(const BaseTransferFunction& o) const override {
			return BaseTransferFunction::equals(o) && o.equals(*this);
		}

		virtual bool equals(const TransferFunction<S>& o) const override {
			return memcmp(transferFunction, o.transferFunction, getArrayLength() * sizeof(TransferFunction<S>::sampleT)) == 0;
		}

		virtual BaseTransferFunction& copy(const BaseTransferFunction& o) override {
			o.copyTo(*this);
			return BaseTransferFunction::copy(o);
		}

		virtual void copyTo(TransferFunction<S>& o) const override {
			memcpy(o.transferFunction, transferFunction, sizeof(o.transferFunction));
			for (int i = 0; i < numChannels(); ++i) {
				o.controlPoints[i] = controlPoints[i];
			}
		}
};

BRWL_NS_END
