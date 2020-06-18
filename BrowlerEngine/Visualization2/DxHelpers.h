#pragma once


void makeStaticSamplerDescription(D3D12_STATIC_SAMPLER_DESC& staticSampler, unsigned int shaderRegister);

enum class PitTex : uint8_t
{
    REFRACTION = 0,
    PARTICLE_COLOR,
    OPACITY,
    MEDIUM,
    MAX,
    MIN = 0
};

void makeSingeSrvDescriptorRange(D3D12_DESCRIPTOR_RANGE& range, unsigned int baseRegister);