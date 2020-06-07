#include "Preintegration.h"

BRWL_RENDERER_NS

namespace
{
	void checkSize(Image& image, unsigned int sideLen) {
		BRWL_EXCEPTION(
			image.isValid() &&
			image.getBufferSize() == (size_t)sideLen * (size_t)sideLen * sizeof(Image::sampleT) &&
			image.getSizeX() == sideLen &&
			image.getSizeY() == sideLen,
			BRWL_CHAR_LITERAL("Invalid image size."));
	}
}

void makePreintegrationTable(Image& image, float* transferFunc, unsigned int lenFunc)
{
	checkSize(image, lenFunc);
	const size_t yStep = image.getSizeX();
	Image::sampleT* const  table = (Image::sampleT*)image.getPtr();
	for (size_t y = 0; y < lenFunc; ++y)
	{
		Image::sampleT* lineStart = table + y * yStep;
		double sum = ((double)transferFunc[y]) * 0.5;
		for (size_t x = y + 1; x < lenFunc; ++x)
		{
			lineStart[x] = (float)(sum + ((double)transferFunc[x]) * 0.5 / (double)x);
			sum += transferFunc[x];
		}
	}

	for (size_t y = 0; y < lenFunc; ++y)
	{
		Image::sampleT* lineStart = table + y * yStep;
		for (size_t x = 0; x < y; ++x)
		{
			lineStart[x] = table[x * yStep + y];
		}
	}
}

void makeDiagram(Image& image, float* transferFunc, unsigned int lenFunc)
{
	checkSize(image, lenFunc);
	const size_t yStep = image.getSizeX();
	Image::sampleT* const  table = (Image::sampleT*)image.getPtr();
	for (size_t y = 0; y < lenFunc; ++y)
	{
		Image::sampleT* lineStart = table + y * yStep;
		for (size_t x = 0; x < lenFunc; ++x)
		{
			lineStart[x] = transferFunc[x] * lenFunc < lenFunc - y ? 0 : 1;
		}
	}
}

void makeTestImage(Image& image, float* transferFunc, unsigned int lenFunc)
{
	checkSize(image, lenFunc);
	const size_t yStep = image.getSizeX();
	Image::sampleT* const  table = (Image::sampleT*)image.getPtr();
	for (size_t y = 0; y < lenFunc; ++y)
	{
		Image::sampleT* lineStart = table + y * yStep;
		for (size_t x = 0; x < lenFunc; ++x)
		{
			lineStart[x] = (double)x;
		}
	}
}



BRWL_RENDERER_NS_END
