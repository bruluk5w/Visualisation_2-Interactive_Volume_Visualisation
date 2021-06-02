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
void makePreintegrationTable(T<S>& image, typename T<S>::sampleT* transferFunc, unsigned int lenFunc)
{
	// TODO: fix 10 bit preintegration texture
	if (SampleFormat::VEC4F32 == S && lenFunc > 512)
		return;

	checkSize<S>(image, lenFunc);
	const size_t yStep = image.getSizeX();
	T<S>::sampleT* const table = static_cast<T<S>::sampleT*>(image.getData());
	for (size_t y = 0; y < lenFunc; ++y)
	{
		T<S>::sampleT* lineStart = table + y * yStep;
		for (size_t x = y; x < lenFunc; ++x)
		{ // integrate from y to x
			T<S>::sampleT sum = T<S>::sampleT();
			for (size_t i = y ; i <= x; ++i) {
				sum += transferFunc[i];
			}

			lineStart[x] =  sum / (float)(x + 1 - y);
			//BRWL_EXCEPTION(lineStart + x < table + (lenFunc*lenFunc), nullptr);

		}
	}

	for (size_t y = 0; y < lenFunc; ++y)
	{
		T<S>::sampleT* lineStart = table + y * yStep;
		for (size_t x = 0; x < y; ++x)
		{
			lineStart[x] = table[x * yStep + y];
			//BRWL_EXCEPTION(lineStart + x < table + (lenFunc * lenFunc), nullptr);
		}
	}
}

template inline void makePreintegrationTable<>(Texture<SampleFormat::F32>& image, float* transferFunc, unsigned int lenFunc);
template inline void makePreintegrationTable<>(Texture<SampleFormat::F64>& image, double* transferFunc, unsigned int lenFunc);
template inline void makePreintegrationTable<>(Texture<SampleFormat::VEC4F32>& image, Vec4* transferFunc, unsigned int lenFunc);


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
