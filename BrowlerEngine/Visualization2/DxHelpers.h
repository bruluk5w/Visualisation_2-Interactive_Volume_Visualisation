#pragma once // (c) 2020 Lukas Brunner


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


// Root SRV parameters for textures are tables with single entries, because the descriptor handles are not located in a continuous range
void makePitSrvRootParameter(D3D12_ROOT_PARAMETER* param, unsigned int n, unsigned int baseRegister, D3D12_SHADER_VISIBILITY visibility);

void makeSingeSrvDescriptorRange(D3D12_DESCRIPTOR_RANGE& range, unsigned int baseRegister);