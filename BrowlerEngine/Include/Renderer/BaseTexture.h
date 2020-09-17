#pragma once // (c) 2020 Lukas Brunner

BRWL_RENDERER_NS


class TextureManager;
struct TextureResource;

//!  The main texture class
/*!
 * The textures have a CPU-side buffer, one "staged" texture and a "live" texture.
 * The staged texture is used for uploading and is then swapped with the live texture once it is known to be fully resident on the GPU.
 * Pixel data is stored as a C contiguous array.
 * This class is not concerened with the actual pixel format. The derived Texture class allows specialization on the pixel format.
 */
class BaseTexture
{
public:

	/*!
	 * \param name The name of the texture. This is will also be set as a name to the respective resource in the graphics API.
	 * \param mgr The TextureManager which will manage this texture. The texture automatically registers itself with the the TextureManager.
	 */
	BaseTexture(const BRWL_CHAR* name, TextureManager* mgr);

	virtual ~BaseTexture();

	/*!
	 * (Re)creates an empty CPU-side buffer for a texture of the the given size.
	 * TODO: effects on GPU texture?
	 */
	void create(uint16_t sizeX, uint16_t sizeY, uint16_t sizeZ = 1);
	/*!
	 * Returns true if the CPU-side buffer exists.
	 */
	bool isValid() const { return valid; }

	/**/
	bool isResident() const { return gpu.isResident(); }
	/*!
	 * Returns a pointer to the start of the internal buffer.
	 * May only be called if that buffer exists.
	 */
	const uint8_t* getData() const { checkValid(); return data.get(); }

	uint16_t getSizeX() const { checkValid(); return sizeX; } //!< The number of colums.
	uint16_t getSizeY() const { checkValid(); return sizeY; } //!< The number of rows.
	uint16_t getSizeZ() const { checkValid(); return sizeZ; } //!< The number image slices.
	uint32_t getStrideX() const { checkValid(); return strideX; } //!< The size of a pixel in bytes.
	uint32_t getStrideY() const { checkValid(); return strideY; } //!< The size of an row in bytes.
	uint32_t getStrideZ() const { checkValid(); return strideZ; } //!< The size of an image slice in bytes.

	size_t getBufferSize() const { checkValid(); return bufferSize; } //!< The size of the whole CPU-side buffer in bytes.
	const BRWL_CHAR* getName() const { return name.c_str(); } //!< The name of the texture. This is also set to graphics API as a name of the corresponding resource.
	
	/*!
	 * Writes all zeros over the CPU-side buffer.
	 */
	void clear();

protected:
	void checkValid() const { BRWL_EXCEPTION(valid, BRWL_CHAR_LITERAL("Accessing data of invalid image.")); }
	virtual int getSampleByteSize() const = 0;


	BRWL_STR name;
	TextureManager* mgr;
	bool valid;
	uint16_t sizeX;
	uint16_t sizeY;
	uint16_t sizeZ;
	uint32_t strideX;
	uint32_t strideY;
	uint32_t strideZ;
	size_t bufferSize;
	std::unique_ptr<uint8_t[]> data;

	struct GpuData
	{
		bool isResident() const;

		ComPtr<ID3D12Fence> fence;
		uint64_t uploadFenceValue;
		HANDLE uploadEvent;
		std::unique_ptr<TextureResource> liveTexture;
		std::unique_ptr<TextureResource> stagedTexture;
	} gpu;
};


BRWL_RENDERER_NS_END
