#pragma once

#include "Image.h"

BRWL_RENDERER_NS

// we expect the image to be null initialized
void makePreintegrationTable(Image& image, float* transferFunc, unsigned int lenFunc)
{
	BRWL_EXCEPTION(
		image.isValid() &&
		image.getBufferSize() == lenFunc * lenFunc * sizeof(Image::sampleT) &&
		image.getSizeX() == lenFunc &&
		image.getSizeY() == lenFunc,
		BRWL_CHAR_LITERAL("Invalid image size."));
	const int yStride = image.getStrideY() / sizeof(Image::sampleT);
	Image::sampleT* const  table = (Image::sampleT*)image.getPtr();
	for (int y = 0; y < lenFunc; ++y)
	{
		Image::sampleT* lineStart = table + y * yStride;
		double sum = ((double)transferFunc[y]) * 0.5;
		for (int x = y + 1; x < lenFunc; ++x)
		{
			lineStart[x] = (float)(sum + ((double)transferFunc[x]) * 0.5 / (double)x);
			sum += transferFunc[y + x];
		}
	}

	for (int y = 0; y < lenFunc; ++y)
	{
		Image::sampleT* lineStart = table + y * yStride;
		for (int x = 0; x < y; ++x)
		{
			lineStart[x] = table[x * yStride + y];
		}
	}
}

BRWL_RENDERER_NS_END