#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/Texture.h"

BRWL_RENDERER_NS


// we expect the image to be null initialized
template<SampleFormat S, template<SampleFormat> typename T = Texture>
void makePreintegrationTable(T<S>& image, typename T<S>::sampleT* transferFunc, unsigned int lenFunc);


// we expect the image to be null initialized
template<SampleFormat S, template<SampleFormat> typename T = Texture>
void makeDiagram(T<S>& image, float* transferFunc, unsigned int lenFunc);


BRWL_RENDERER_NS_END