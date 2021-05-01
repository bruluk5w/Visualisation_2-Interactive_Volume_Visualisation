#include "DataSet.h"

#include <fstream>
#include <filesystem>

//#define SWAP_16(s) ( ((s) >> 8) | ((s) << 8) )

BRWL_RENDERER_NS


template<SampleFormat S>
void DataSet<S>::loadFromFile(const BRWL_CHAR* relativePath)
{
	this->valid = false;
	size_t size = 0;
	try
	{
		size = std::filesystem::file_size(relativePath);
	}
	catch (std::filesystem::filesystem_error& e)
	{
		BRWL_CHAR buf[256];
		BRWL_SNPRINTF(buf, BRWL::countof(buf), BRWL_CHAR_LITERAL("Failed to get file size of asset: %s\nError:\n%hs"), relativePath.c_str(), e.what());
		BRWL_EXCEPTION(false, buf);
		return;
	}

	sourcePath = relativePath;
	std::ifstream file(sourcePath.c_str(), std::ifstream::binary);
	if (!BRWL_VERIFY(file.is_open(), BRWL_CHAR_LITERAL("Failed to open asset file")))
	{
		sourcePath = nullptr;
		return;
	}

	if (!BRWL_VERIFY(size >= 6, BRWL_CHAR_LITERAL("File too short for header.")))
	{
		sourcePath = nullptr;
		return;
	}
	uint16_t sizeX, sizeY, sizeZ;
	file.read((char*)&sizeX, sizeof(sizeX));
	file.read((char*)&sizeY, sizeof(sizeY));
	file.read((char*)&sizeZ, sizeof(sizeZ));
	this->create(sizeX, sizeY, sizeZ, TextureDimension::TEXTURE_3D, TextureCreationParams::NONE);


	if (!BRWL_VERIFY(this->size - sizeof(this->sizeX) - sizeof(this->sizeY) - sizeof(this->sizeZ) >= this->bufferSize, BRWL_CHAR_LITERAL("File too short for data content.")))
	{
		sourcePath = nullptr;
		this->bufferSize = this->strideX = this->strideY = this->strideZ = this->sizeX = this->sizeY = this->sizeZ = 0;
		bbox = {};
		return;
	}

	bbox = {
		{-(float)this->sizeX * 0.5f, -(float)this->sizeY * 0.5f, -(float)this->sizeZ * 0.5f},
		{(float)this->sizeX * 0.5f, (float)this->sizeY * 0.5f, (float)this->sizeZ * 0.5f } };

	this->data = std::unique_ptr<uint8_t[]>(new uint8_t[this->bufferSize]);
	file.read((char*)this->data.get(), this->bufferSize);

	this->valid = true;
}


BRWL_RENDERER_NS_END
