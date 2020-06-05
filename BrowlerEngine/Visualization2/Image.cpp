#include "Image.h"

BRWL_RENDERER_NS

//const Image::sampleT Image::clearColor = Image::sampleT();

Image::Image(BRWL_STR name) :
	name(std::move(name)),
	valid(false),
	sizeX(0),
	sizeY(0),
	strideX(0),
	strideY(0),
	bufferSize(0),
	data(nullptr)
{ }

void Image::create(uint16_t sizeX, uint16_t sizeY)
{
	valid = false;

	this->sizeX = sizeX;
	this->sizeY = sizeY;
	strideX = sizeof(sampleT);
	strideY = sizeX * strideX;
	bufferSize = strideY * sizeY;

	data = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
	clear();
	valid = true;
}

void Image::clear()
{
	memset(data.get(), 0, bufferSize);
}


BRWL_RENDERER_NS_END