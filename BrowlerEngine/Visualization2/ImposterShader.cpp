#include "ImposterShader.h"

#include "ImposterShader_vs_vs.h"
#include "ImposterShader_ps_ps.h"

#include "DxHelpers.h"
#include "Renderer/Renderer.h"
#include "Renderer/PAL/d3dx12.h"



BRWL_RENDERER_NS


ImposterShader::ImposterShader(ID3D12Device* device, const D3D12_INPUT_LAYOUT_DESC& inputLayout) :
    rootSignature(nullptr),
    pipelineState(nullptr)
{
    destroy();
    // building pipeline state object and rootsignature

    {
        D3D12_ROOT_PARAMETER param[3];
        memset(&param, 0, sizeof(param));
        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues = VsConstants::num32BitValues;
        param[0].Constants.ShaderRegister = 0;
        param[0].Constants.RegisterSpace = 0;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[1].Constants.Num32BitValues = PsConstants::num32BitValues;
        param[1].Constants.ShaderRegister = 1;
        param[1].Constants.RegisterSpace = 0;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // color texture
        D3D12_DESCRIPTOR_RANGE srvRange;
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0;
        srvRange.RegisterSpace = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].DescriptorTable.NumDescriptorRanges = 1;
        param[2].DescriptorTable.pDescriptorRanges = &srvRange;
        param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // color tex sampler
        D3D12_STATIC_SAMPLER_DESC staticSampler;
        makeStaticSamplerDescription(staticSampler, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        memset(&rootSignatureDesc, 0, sizeof(rootSignatureDesc));
        rootSignatureDesc.NumParameters = countof(param);
        rootSignatureDesc.pParameters = param;
        rootSignatureDesc.NumStaticSamplers = 1;
        rootSignatureDesc.pStaticSamplers = &staticSampler;
        rootSignatureDesc.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        ComPtr<ID3DBlob> blob = nullptr;
        BRWL_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, NULL)), BRWL_CHAR_LITERAL("Failed to serialize root signature."));

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))), BRWL_CHAR_LITERAL("Failed to create root signature")))
        {
            destroy();
            return;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        memset(&psoDesc, 0, sizeof(psoDesc));
        psoDesc.NodeMask = 0;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.NumRenderTargets = 3;
        psoDesc.RTVFormats[0] = PAL::g_RenderTargetFormat;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        psoDesc.VS = { Imposter_vs_vs, sizeof(Imposter_vs_vs) };
        psoDesc.PS = { Imposter_ps_ps, sizeof(Imposter_ps_ps) };
        psoDesc.InputLayout = inputLayout;


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
            desc.FillMode = D3D12_FILL_MODE_WIREFRAME;
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

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))), BRWL_CHAR_LITERAL("Failed to create pipeline state for imposter shader.")))
        {
            destroy();
            return;
        }
    }
}

void ImposterShader::destroy()
{
    rootSignature = nullptr;
    pipelineState = nullptr;
}

ImposterShader::~ImposterShader()
{
    destroy();
}

void ImposterShader::setupDraw(ID3D12GraphicsCommandList* cmd, const ImposterShader::VsConstants& vsConstants, const ImposterShader::PsConstants psConstants, const PAL::DescriptorHandle::NativeHandles& outColorBufferDescriptorHandle)
{
    SCOPED_GPU_EVENT(cmd, 0, 255, 0, "Imposter Shader");

    cmd->SetPipelineState(pipelineState.Get());
    cmd->SetGraphicsRootSignature(rootSignature.Get());

    // constants
    cmd->SetComputeRoot32BitConstants(0, VsConstants::num32BitValues, &vsConstants, 0);
    cmd->SetComputeRoot32BitConstants(0, PsConstants::num32BitValues, &psConstants, 0);

    // set color texture
    cmd->SetComputeRootDescriptorTable(0, outColorBufferDescriptorHandle.residentGpu);
}

BRWL_RENDERER_NS_END