#include "BaseTexture.h"
#include "TextureResource.h"
#include "TextureManager.h"

BRWL_RENDERER_NS


BaseTexture::BaseTexture(const BRWL_CHAR* name) :
	name(name),
	valid(false),
	dim(TextureDimension::MAX),
	sizeX(0),
	sizeY(0),
	sizeZ(0),
	strideX(0),
	strideY(0),
	strideZ(0),
	bufferSize(0),
	data(nullptr)
{ }

void BaseTexture::create(uint16_t sizeX, uint16_t sizeY, uint16_t sizeZ, TextureDimension dim, TextureCreationParams params)
{	
	BRWL_EXCEPTION(sizeZ <= 1 || dim == TextureDimension::TEXTURE_3D, BRWL_CHAR_LITERAL("Multiple image slice require a 3d texture."));

	valid = false;
	this->dim = dim;
	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;
	strideX = getSampleByteSize();
	strideY = sizeX * strideX;
	strideZ = sizeZ * strideY;
	bufferSize = strideZ * sizeZ;

	data = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
	if (!!(params & TextureCreationParams::INIT_ZERO_MEMORY))
	{
		zero();
	}

	valid = true;
}

void BaseTexture::zero()
{
	if (data)
	{
		memset(data.get(), 0, bufferSize);
	}
}


BRWL_RENDERER_NS_END
