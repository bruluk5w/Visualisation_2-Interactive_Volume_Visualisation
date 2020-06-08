#include "MainShader.h"

#include "VertexShader_vs.h"
#include "PixelShader_ps.h"

#include "Renderer/PAL/d3dx12.h"

namespace {
    struct DrawData {
        ::BRWL::Vec3 pos;
        ::BRWL::Vec2 uv;
        ::BRWL::Vec4 col;
    };
    const DrawData quad[] = {
        { { 0, 0, 0},{0,0},{1.0f, 1.0f, 1.0f, 1.0f}},
        { { 1, 0, 0},{0,0},{1.0f, 1.0f, 1.0f, 1.0f}},
        { { 1, 1, 0},{0,0},{1.0f, 1.0f, 1.0f, 1.0f}},
        { { 1, 0, 0},{0,0},{1.0f, 1.0f, 1.0f, 1.0f}},
        { { 1, 1, 0},{0,0},{1.0f, 1.0f, 1.0f, 1.0f}},
        { { 1, 0, 0},{0,0},{1.0f, 1.0f, 1.0f, 1.0f}}
    };
}

BRWL_RENDERER_NS


MainShader::MainShader() :
    rootSignature(nullptr),
    pipelineState(nullptr),
    initialized(false)
{ }

bool MainShader::create(ID3D12Device* device)
{
    if (initialized) return true;

    {
        D3D12_STATIC_SAMPLER_DESC staticSampler;
        memset(&staticSampler, 0, sizeof(staticSampler));
        staticSampler.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        staticSampler.MipLODBias = 0.f;
        staticSampler.MaxAnisotropy = 0;
        staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        staticSampler.MinLOD = 0.f;
        staticSampler.MaxLOD = 0.f;
        staticSampler.ShaderRegister = 0;
        staticSampler.RegisterSpace = 0;
        staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // Mainly for the texture
        D3D12_DESCRIPTOR_RANGE descRange;
        memset(&descRange, 0, sizeof(descRange));
        descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRange.NumDescriptors = 1;
        descRange.BaseShaderRegister = 0;
        descRange.RegisterSpace = 0;
        descRange.OffsetInDescriptorsFromTableStart = 0;

        D3D12_ROOT_PARAMETER param[2] = {};
        memset(&param, 0, sizeof(param));

        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.ShaderRegister = 0;
        param[0].Constants.RegisterSpace = 0;
        param[0].Constants.Num32BitValues = 16;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges = &descRange;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        memset(&rootSignatureDesc, 0, sizeof(param));
        rootSignatureDesc.NumParameters = _countof(param);
        rootSignatureDesc.pParameters = param;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.pStaticSamplers = &staticSampler;
        rootSignatureDesc.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


        ComPtr<ID3DBlob> blob = nullptr;
        if (!BRWL_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, NULL)), BRWL_CHAR_LITERAL("Failed to serialize root signature.")))
        {
            destroy();
            return false;
        }

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()))), BRWL_CHAR_LITERAL("Failed to create root signature")))
        {
            destroy();
            return false;
        }
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.NodeMask = 1;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = PAL::g_RenderTargetFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    psoDesc.VS = { VertexShader_vs_vs, sizeof(VertexShader_vs_vs) };
    psoDesc.PS = { PixelShader_ps_ps, sizeof(PixelShader_ps_ps) };

    // Create the input layout
    static D3D12_INPUT_ELEMENT_DESC local_layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, (UINT)offsetof(DrawData, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)offsetof(DrawData, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(DrawData, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    psoDesc.InputLayout = { local_layout, 3 };


    // Create the blending setup
    {
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        /*D3D12_BLEND_DESC& desc = psoDesc.BlendState;
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;*/
    }

    // Create the rasterizer state
    {
        D3D12_RASTERIZER_DESC& desc = psoDesc.RasterizerState;
        desc.FillMode = D3D12_FILL_MODE_SOLID;
        desc.CullMode = D3D12_CULL_MODE_NONE;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        desc.DepthClipEnable = true;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;
        desc.ForcedSampleCount = 0;
        desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }

    // Create depth-stencil State
    {
        D3D12_DEPTH_STENCIL_DESC& desc = psoDesc.DepthStencilState;
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        desc.StencilEnable = false;
        desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        desc.BackFace = desc.FrontFace;
    }

    if (!BRWL_VERIFY(SUCCEEDED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.GetAddressOf()))), BRWL_CHAR_LITERAL("Failed to create pipeline state.")))
    {
        destroy();
        return false;
    }

    initialized = true;
    return initialized;
}

void MainShader::render()
{
    if (!initialized) return;
}

void MainShader::destroy()
{
    if (!initialized) return;
    pipelineState = nullptr;
    rootSignature = nullptr;
}

BRWL_RENDERER_NS_END