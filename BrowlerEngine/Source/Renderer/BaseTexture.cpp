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
	gpu()
{
	BRWL_EXCEPTION(mgr, BRWL_CHAR_LITERAL("TextureManager is nullptr. A TextureManager has to be provided for each Texture instance."));
	mgr->add(this);
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

void BaseTexture::requestUpload()
{
	BRWL_EXCEPTION(isReadyForUpload(), BRWL_CHAR_LITERAL("TextureResource was not in the expected state."));
	gpu.stagedTexture->state = TextureResource::State::REQUESTING_UPLOAD;
}

bool BaseTexture::isReadyForUpload()
{
	return gpu.stagedTexture->state == TextureResource::State::NONE;
}

bool BaseTexture::initGpu()
{
	ID3D12Device* device = mgr->getDevice();
	return gpu.init(device);
}

void BaseTexture::destroyGpu()
{
	gpu.destroy();
}

BaseTexture::GpuData::~GpuData()
{
	BRWL_EXCEPTION(!liveTexture && !stagedTexture && !fence && !uploadEvent,
		BRWL_CHAR_LITERAL("Resources have not been properly destroyed.")
	);
}

bool BaseTexture::GpuData::init(ID3D12Device* device)
{
	if (!BRWL_VERIFY(SUCCEEDED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))), BRWL_CHAR_LITERAL("Failed to create pitTexture fence.")))
	{
		destroy();
		return false;
	}

	uploadEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual reset TRUE
	if (!BRWL_VERIFY(uploadEvent != NULL, BRWL_CHAR_LITERAL("Failed to create upload event.")))
	{
		destroy();
		return false;
	}

	liveTexture = std::make_unique<TextureResource>();
	stagedTexture = std::make_unique<TextureResource>();

	return true;
}

void BaseTexture::GpuData::destroy()
{
	if (liveTexture)
	{
		liveTexture->destroy();
		liveTexture = nullptr;
	}
	if (stagedTexture)
	{
		stagedTexture->destroy();
		stagedTexture = nullptr;

	}
	
	fence = nullptr;
	
	if (uploadEvent)
	{
		CloseHandle(uploadEvent);
		uploadEvent = NULL;
	}
}
bool BaseTexture::GpuData::isResident() const
{
	return liveTexture && liveTexture->isResident();
}


BRWL_RENDERER_NS_END
