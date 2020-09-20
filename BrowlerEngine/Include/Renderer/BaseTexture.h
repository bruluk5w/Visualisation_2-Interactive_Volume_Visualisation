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
 * The texture classes and texture manager are not thread safe and while texture uploads are queued on a copy queue CPU work is still done on the calling thread.
 * The usage is intended as follows:
 * 1. Create a CPU-side buffer via BaseTexture::create and fill the buffer with texture data.
 * 2. Call BaseTexture::requestUpload. This will lock the CPU-side buffer and from then on until the upload is finished only the TextureManager may modify the texture.
 * 3. 
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
	 * \return Returns true if the CPU-side buffer exists.
	 */
	bool isValid() const { return valid; }

	/**/
	bool isResident() const { return gpu.isResident(); }
	/*!
	 * May only be called if that buffer exists.
	 * \return Returns a pointer to the start of the internal buffer.
	 */
	const uint8_t* getPtr() const { checkValid(); return data.get(); }

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

	void requestUpload();

	bool isReadyForUpload();

	/*!
	 * Creates the respective graphics resources required to upload and use the texture on the GPU.
	 * \return Returns true if the initialization was successful, else false;
	 */
	virtual bool initGpu();
	virtual void destroyGpu();


	class GpuData
	{
	private:

		friend class BaseTexture;
		friend class TextureManager;
		virtual ~GpuData();
		/*!
		 * Creates the respective graphics resources required to upload and use the texture on the GPU.
		 * \param device The device to create the resource for.
		 * \return Returns true if the initialization was successful, else false;
		 */
		bool init(ID3D12Device* device);
		void destroy();
		bool isResident() const;
		ComPtr<ID3D12Fence> fence =  nullptr ;
		uint64_t uploadFenceValue = 0;
		HANDLE uploadEvent = NULL;
		std::unique_ptr<TextureResource> liveTexture = nullptr;
		std::unique_ptr<TextureResource> stagedTexture = nullptr;
	} gpu;

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
};


BRWL_RENDERER_NS_END
