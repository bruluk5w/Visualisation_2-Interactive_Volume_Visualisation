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

void makePitSrvRootParameter(D3D12_ROOT_PARAMETER* param, unsigned int n, unsigned int baseRegister, D3D12_SHADER_VISIBILITY visibility)
{
    BRWL_EXCEPTION(n >= ENUM_CLASS_TO_NUM(PitTex::MAX), nullptr);
    D3D12_DESCRIPTOR_RANGE descriptorRanges[ENUM_CLASS_TO_NUM(PitTex::MAX)];
    memset(&descriptorRanges, 0, ENUM_CLASS_TO_NUM(PitTex::MAX) * sizeof(D3D12_DESCRIPTOR_RANGE));
    for (int i = 0; i <1 /* ENUM_CLASS_TO_NUM(PitTex::MAX)*/; ++i)
    {
        D3D12_DESCRIPTOR_RANGE& range = descriptorRanges[i];
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = 1;
        range.BaseShaderRegister = baseRegister + i;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    }

    for (int i = 0; i < 1 /* ENUM_CLASS_TO_NUM(PitTex::MAX)*/; ++i)
    {
        param[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[i].DescriptorTable.NumDescriptorRanges = 1;
        param[i].DescriptorTable.pDescriptorRanges = &descriptorRanges[i];
        param[i].ShaderVisibility = visibility;
    }
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
