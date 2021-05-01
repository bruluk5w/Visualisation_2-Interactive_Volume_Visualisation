#pragma once // (c) 2020 Lukas Brunner

#include "BaseTexture.h"

BRWL_RENDERER_NS


//!  The main texture class
/*!
 * This class allows specialization on the pixel format of a texture.
 * The template parameter states the type of one sample and may be a struct (possibly including bitfields) for multiple channels.
 */
template<typename T>
class Texture : public BaseTexture
{
	friend class BaseTextureManager;

	/*!
	 * Textures are only created by a texture manager. This is will also be set as a name to the respective resource in the graphics API.
	 * \param name The name of the texture. This is will also be set as a name to the respective resource in the graphics API.
	 */
	Texture(const BRWL_CHAR* name) : BaseTexture(name)
	{ }

public:

	/*!
	 * The type of one sample/pixel. For multiple channels this is supposed to be a struct.
	 */
	using sampleT = T;

	/*!
	 * Returns a pointer to the start of the internal buffer.
	 * May only be called if that buffer exists.
	 */
	const sampleT* getData() const { checkValid(); return data.get(); }

	//! Inherited via BaseTexture
	virtual int getSampleByteSize() const override
	{
		return sizeof(sampleT);
	}
};

typedef Texture<float> TextureF32;
typedef Texture<double> TextureF64;
typedef Texture<int16_t> TextureS16;
typedef Texture<uint16_t> TextureU16;
typedef Texture<int32_t> TextureS32;
typedef Texture<uint32_t> TextureU32;


BRWL_RENDERER_NS_END
