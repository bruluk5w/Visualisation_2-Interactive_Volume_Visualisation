#include "DataSet.h"

#include <fstream>
#include <filesystem>

#define SWAP_16(s) ( ((s) >> 8) | ((s) << 8) )

DataSet::DataSet(BRWL_STR name) :
	name(std::move(name)),
	sourcePath(),
	valid(false),
	sizeX(0),
	sizeY(0),
	sizeZ(0),
	strideX(0),
	strideY(0),
	strideZ(0),
	bufferSize(0),
	data(nullptr)
{ }

void DataSet::loadFromFile(BRWL_STR relativePath)
{
	valid = false;
	size_t size = 0;
	try {
		 size = std::filesystem::file_size(relativePath.c_str()); // attempt to get size of a directory
	}
	catch (std::filesystem::filesystem_error& e)
	{
		//BRWL_STR str(e.what(), e.what() + strlen(e.what()));
		BRWL_CHAR buf[256];
		BRWL_SNPRINTF(buf, BRWL::countof(buf), BRWL_CHAR_LITERAL("Failed to get file size of asset: %s\nError:\n%s"), name.c_str(), e.what());
		BRWL_EXCEPTION(false, buf);
		return;
	}

	sourcePath = std::move(relativePath);
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

	file.read((char*)&sizeX, sizeof(sizeX));
	file.read((char*)&sizeY, sizeof(sizeY));
	file.read((char*)&sizeZ, sizeof(sizeZ));
	strideX = sizeof(sampleT);
	strideY = sizeX * strideX;
	strideZ = strideY * sizeY;
	bufferSize = strideZ * sizeZ;

	if (!BRWL_VERIFY(size - 6 >= bufferSize, BRWL_CHAR_LITERAL("File too short for data content.")))
	{
		sourcePath = nullptr;
		sizeX = sizeY = sizeZ = strideX = strideY = strideZ = bufferSize = 0;
		return;
	}

	data = std::unique_ptr<uint8_t[]>(new uint8_t[bufferSize]);
	for (int z = 0; z < sizeZ; ++z) {
		size_t offsetZ = z * strideZ;
		/*for (int y = 0; y < sizeY; ++y) {
			size_t offsetY = offsetZ + y * strideY;
			for (int x = 0; x < sizeX; ++x) {
				file.read((char*)&data[offsetY + x * strideX], sizeof(sampleT));
			}
		}*/

		file.read((char*)&data[offsetZ], strideZ);
	}

	valid = true;
}
