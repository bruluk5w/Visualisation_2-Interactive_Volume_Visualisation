#pragma once // (c) 2020 Lukas Brunner


void makeStaticSamplerDescription(D3D12_STATIC_SAMPLER_DESC& staticSampler, unsigned int shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

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