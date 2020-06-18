#include "DxHelpers.h"

void makeStaticSamplerDescription(D3D12_STATIC_SAMPLER_DESC& staticSampler, unsigned int shaderRegister)
{
    memset(&staticSampler, 0, sizeof(staticSampler));
    staticSampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    staticSampler.MipLODBias = 0.f;
    staticSampler.MaxAnisotropy = 0;
    staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    staticSampler.MinLOD = 0.f;
    staticSampler.MaxLOD = 0.f;
    staticSampler.ShaderRegister = shaderRegister;
    staticSampler.RegisterSpace = 0;
    staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void makeSingeSrvDescriptorRange(D3D12_DESCRIPTOR_RANGE& range, unsigned int baseRegister)
{
    memset(&range, 0, sizeof(range));
    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = 1;
    range.BaseShaderRegister = baseRegister;
    range.RegisterSpace = 0;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}
