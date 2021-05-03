#include "Preintegration.h"

#include "Renderer/Texture.h"

BRWL_RENDERER_NS

namespace
{
	template<SampleFormat S>
	void checkSize(Texture<S>& image, unsigned int sideLen)
	{
		BRWL_EXCEPTION(
			image.isValid() &&
			image.getBufferSize() == (size_t)sideLen * (size_t)sideLen * sizeof(Texture<S>::sampleT) &&
			image.getSizeX() == sideLen &&
			image.getSizeY() == sideLen,
			BRWL_CHAR_LITERAL("Invalid image size."));
	}
}

// todo: remove?
//
//void makePreintegrationTable(Image& image, float* transferFunc, unsigned int lenFunc)
//{
//	checkSize(image, lenFunc);
//	const size_t yStep = image.getSizeX();
//	Image::sampleT* const  table = (Image::sampleT*)image.getPtr();
//	for (size_t y = 0; y < lenFunc; ++y)
//	{
//		Image::sampleT* lineStart = table + y * yStep;
//		double sum = ((double)transferFunc[y]) * 0.5;
//		for (size_t x = y + 1; x < lenFunc; ++x)
//		{
//			lineStart[x] = (float)(sum + ((double)transferFunc[x]) * 0.5 / (double)x);
//			sum += transferFunc[x];
//		}
//	}
//
//	for (size_t y = 0; y < lenFunc; ++y)
//	{
//		Image::sampleT* lineStart = table + y * yStep;
//		for (size_t x = 0; x < y; ++x)
//		{
//			lineStart[x] = table[x * yStep + y];
//		}
//	}
//
//}

template<SampleFormat S, template<SampleFormat> typename T>
void makePreintegrationTable(T<S>& image, float* transferFunc, unsigned int lenFunc)
{
	checkSize<S>(image, lenFunc);
	const size_t yStep = image.getSizeX();
	T<S>::sampleT* const  table = (T<S>::sampleT*)image.getPtr();
	for (size_t y = 0; y < lenFunc; ++y)
	{
		T<S>::sampleT* lineStart = table + y * yStep;
		for (size_t x = y + 1; x < lenFunc; ++x)
		{ // integrate from y to x
			double sum = 0;
			for (int i = y + 1; i < x; ++i) {
				sum += (transferFunc[i - 1] + transferFunc[i]) * 0.5;
			}

			lineStart[x] = (T<S>::sampleT)sum / (x - y);
		}
	}

	for (size_t y = 0; y < lenFunc; ++y)
	{
		T<S>::sampleT* lineStart = table + y * yStep;
		for (size_t x = 0; x < y; ++x)
		{
			lineStart[x] = table[x * yStep + y];
		}
	}
}

template inline void makePreintegrationTable<>(Texture<SampleFormat::F32>& image, float* transferFunc, unsigned int lenFunc);
template inline void makePreintegrationTable<>(Texture<SampleFormat::F64>& image, float* transferFunc, unsigned int lenFunc);


template<SampleFormat S, template<SampleFormat> typename T>
void makeDiagram(T<S>& image, float* transferFunc, unsigned int lenFunc)
{
	checkSize<S>(image, lenFunc);
	const size_t yStep = image.getSizeX();
	T<S>::sampleT* const  table = (Texture<S>::sampleT*)image.getPtr();
	for (size_t y = 0; y < lenFunc; ++y)
	{
		T<S>::sampleT* lineStart = table + y * yStep;
		for (size_t x = 0; x < lenFunc; ++x)
		{
			lineStart[x] = transferFunc[x] * (float)lenFunc < lenFunc - y ? 0.f : 1.f;
		}
	}
}

template inline void makeDiagram<>(Texture<SampleFormat::F32>& image, float* transferFunc, unsigned int lenFunc);
template inline void makeDiagram<>(Texture<SampleFormat::F64>& image, float* transferFunc, unsigned int lenFunc);

BRWL_RENDERER_NS_END
