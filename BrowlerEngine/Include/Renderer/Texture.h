#pragma once // (c) 2020 Lukas Brunner

#include "BaseTexture.h"

BRWL_RENDERER_NS


template<SampleFormat S> struct SampleFormatTrait {
	// some type trait prevents the static assertion from being triggered
	static_assert(S && false, "No sample type defined for sample type enum.");
};

template<> struct SampleFormatTrait<SampleFormat::F32> {
	using type = float;
};

template<> struct SampleFormatTrait<SampleFormat::F64> {
	using type = double;
};

template<> struct SampleFormatTrait<SampleFormat::S16> {
	using type = int16_t;
};

template<> struct SampleFormatTrait<SampleFormat::U16> {
	using type = uint16_t;
};

template<> struct SampleFormatTrait<SampleFormat::S32> {
	using type = int32_t;
};

template<> struct SampleFormatTrait<SampleFormat::U32> {
	using type = uint32_t;
};


template<> struct SampleFormatTrait<SampleFormat::VEC4F32> {
	using type = Vec4;
};


//!  The main texture class
/*!
 * This class allows specialization on the pixel format of a texture.
 * The template parameter states the type of one sample and may be a struct (possibly including bitfields) for multiple channels.
 */
template<SampleFormat S>
class Texture : public BaseTexture
{
	friend class BaseTextureManager;

protected:
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
	using sampleT = typename SampleFormatTrait<S>::type;
	static const SampleFormat sampleFormat = S;

	/*!
	 * Returns a pointer to the start of the internal buffer.
	 * May only be called if that buffer exists.
	 */
	const sampleT* getData() const { checkValid(); return data.get(); }

	//! Inherited via BaseTexture
	virtual int getSampleByteSize() const override
	{
		return sampleByteSize();
	}

	static constexpr int sampleByteSize() { return sizeof(sampleT); }

	virtual SampleFormat getSampleFormat() const override
	{
		return S;
	}
};

typedef Texture<SampleFormat::F32> TextureF32;
typedef Texture<SampleFormat::F64> TextureF64;
typedef Texture<SampleFormat::S16> TextureS16;
typedef Texture<SampleFormat::U16> TextureU16;
typedef Texture<SampleFormat::S32> TextureS32;
typedef Texture<SampleFormat::U32> TextureU32;
typedef Texture<SampleFormat::VEC4F32> TextureVEC4F32;


BRWL_RENDERER_NS_END
