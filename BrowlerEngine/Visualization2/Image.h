#pragma once // (c) 2020 Lukas Brunner

BRWL_RENDERER_NS

class Image
{
public:
	using sampleT = float;
	//static const sampleT clearColor;
	Image(BRWL_STR name);
	void create(uint16_t sizeX, uint16_t sizeY);
	bool isValid() const { return valid; }
	const uint8_t* getData() const { checkValid(); return data.get(); }
	uint8_t* getPtr() const { checkValid(); return data.get(); }
	uint16_t getSizeX() const { checkValid(); return sizeX; }
	uint16_t getSizeY() const { checkValid(); return sizeY; }
	uint32_t getStrideX() const { checkValid(); return strideX; }
	uint32_t getStrideY() const { checkValid(); return strideY; }
	size_t getBufferSize() const { checkValid(); return bufferSize; }
	const BRWL_CHAR* getName() const { return name.c_str(); }
	void clear();

private:
	void checkValid() const { BRWL_EXCEPTION(valid, BRWL_CHAR_LITERAL("Accessing data of invalid image.")); }
	BRWL_STR name;

	bool valid;
	uint16_t sizeX;
	uint16_t sizeY;
	uint32_t strideX;
	uint32_t strideY;
	size_t bufferSize;
	std::unique_ptr<uint8_t[]> data;
};

BRWL_RENDERER_NS_END