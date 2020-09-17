#include "BaseTexture.h"
#include "TextureResource.h"
#include "TextureManager.h"

BRWL_RENDERER_NS


BaseTexture::BaseTexture(const BRWL_CHAR* name, TextureManager* mgr) :
	name(name),
	mgr(mgr),
	valid(false),
	sizeX(0),
	sizeY(0),
	sizeZ(0),
	strideX(0),
	strideY(0),
	strideZ(0),
	bufferSize(0),
	data(nullptr),
	gpu{
		nullptr,
		0,
		NULL,
		nullptr,
		nullptr,
}
{
	BRWL_EXCEPTION(mgr, BRWL_CHAR_LITERAL("TextureManager is nullptr. A TextureManager has to be provided for each Texture instance."));
	mgr->add(this);
	gpu.liveTexture = std::make_unique<TextureResource>();
	gpu.stagedTexture = std::make_unique<TextureResource>();
}

BaseTexture::~BaseTexture()
{
	BRWL_EXCEPTION(mgr, BRWL_CHAR_LITERAL("TextureManager may not be nullptr."));
	mgr->remove(this);
}

void BaseTexture::create(uint16_t sizeX, uint16_t sizeY, uint16_t sizeZ/*=1*/)
{
	valid = false;

	this->sizeX = sizeX;
	this->sizeY = sizeY;
	this->sizeZ = sizeZ;
	strideX = getSampleByteSize();
	strideY = sizeX * strideX;
	strideZ = sizeZ * strideY;
	bufferSize = strideZ * sizeZ;

	data = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
	clear();
	valid = true;
}

void BaseTexture::clear()
{
	if (data)
	{
		memset(data.get(), 0, bufferSize);
	}
}

bool BaseTexture::GpuData::isResident() const
{
	return liveTexture && liveTexture->isResident();
}


BRWL_RENDERER_NS_END
