#pragma once // (c) 2020 Lukas Brunner

class DataSet
{
public:
	using sampleT = int16_t;
	DataSet(BRWL_STR name);
	void loadFromFile(BRWL_STR relativePath);
	bool isValid() const { return valid; }
	const uint8_t* getData() const { checkValid(); return data.get(); }
	uint16_t getSizeX() const { checkValid(); return sizeX; }
	uint16_t getSizeY() const { checkValid(); return sizeY; }
	uint16_t getSizeZ() const { checkValid(); return sizeZ; }
	uint64_t getStrideX() const { checkValid(); return strideX; }
	uint64_t getStrideY() const { checkValid(); return strideY; }
	uint64_t getStrideZ() const { checkValid(); return strideZ; }
	size_t getBufferSize() const { checkValid(); return bufferSize; }
	const BRWL_CHAR* getName() const { return name.c_str(); }
private:
	void checkValid() const { BRWL_EXCEPTION(valid, BRWL_CHAR_LITERAL("Accessing data of invalid asset.")); }
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