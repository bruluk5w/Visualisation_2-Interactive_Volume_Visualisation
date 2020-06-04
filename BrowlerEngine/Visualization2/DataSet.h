#pragma once

class DataSet
{
public:
	using sampleT = uint16_t;
	DataSet(BRWL_STR name);
	void loadFromFile(BRWL_STR relativePath);
	bool isValid() const { return valid; }
	const uint8_t* getData() const { BRWL_EXCEPTION(isValid(), BRWL_CHAR_LITERAL("Accessing data of invalid asset.")); return data.get(); }
private:
	BRWL_STR name;
	BRWL_STR sourcePath;

	bool valid;
	uint16_t sizeX;
	uint16_t sizeY;
	uint16_t sizeZ;
	uint64_t strideX;
	uint64_t strideY;
	uint64_t strideZ;
	size_t bufferSize;
	std::unique_ptr<uint8_t[]> data;
};