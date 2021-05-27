#include "GuiTransferFunction.h"


BRWL_NS


const char* BaseTransferFunction::bitDepthNames[] = {
	"8 Bit",
	"10 Bit"
};

BaseTransferFunction::BaseTransferFunction() : 
	bitDepth(BitDepth::BIT_DEPTH_8_BIT),
	textureID(nullptr)
{ }

unsigned int BaseTransferFunction::getArrayLength() const
{
	switch (bitDepth) {
	case BitDepth::BIT_DEPTH_8_BIT:
		return 1 << 8;
	case BitDepth::BIT_DEPTH_10_BIT:
		return 1 << 10;
	default:
		BRWL_UNREACHABLE();
	}

	return 0;
}

bool BaseTransferFunction::equals(const BaseTransferFunction& o) const
{
	return bitDepth == o.bitDepth;
};

bool BaseTransferFunction::equals(const TransferFunction<RENDERER::SampleFormat::F32>& o) const
{
	return false;
}

bool BaseTransferFunction::equals(const TransferFunction<RENDERER::SampleFormat::VEC4F32>& o) const
{
	return false;
}

BaseTransferFunction& BaseTransferFunction::copy(const BaseTransferFunction& o)
{
	bitDepth = o.bitDepth;
	return *this;
}

void BaseTransferFunction::copyTo(TransferFunction<RENDERER::SampleFormat::F32>& o) const
{
	BRWL_EXCEPTION(false, BRWL_CHAR_LITERAL("Cannot copy a transfer function to a different type."));
}

void BaseTransferFunction::copyTo(TransferFunction<RENDERER::SampleFormat::VEC4F32>& o) const
{
	BRWL_EXCEPTION(false, BRWL_CHAR_LITERAL("Cannot copy a transfer function to different type."));
}


BRWL_NS_END
