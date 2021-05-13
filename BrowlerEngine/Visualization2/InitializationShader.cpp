#include "InitializationShader.h"

#include "Initialization_cs_cs.h"

#include "ComputeBuffers.h"

BRWL_RENDERER_NS


InitializationShader::InitializationShader(ID3D12Device* device) :
    rootSignature(nullptr),
    pipelineState(nullptr)
{
    destroy();

    D3D12_DESCRIPTOR_RANGE uavDescRange;
    memset(&uavDescRange, 0, sizeof(uavDescRange));
    uavDescRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    uavDescRange.NumDescriptors = ComputeBuffers::numBuffers;
    uavDescRange.BaseShaderRegister = 0; // register u0 - u5
    uavDescRange.RegisterSpace = 0;
    uavDescRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER param[2];
    memset(&param, 0, sizeof(param));
    // Model- & ViewProjection matrix & voxelsPerCm
    param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param[0].Constants.ShaderRegister = 0; // register b0
    param[0].Constants.RegisterSpace = 0;
    param[0].Constants.Num32BitValues = ShaderConstants::num32BitValues;
    param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param[1].DescriptorTable.NumDescriptorRanges = 1;
    param[1].DescriptorTable.pDescriptorRanges = &uavDescRange;
    param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    memset(&rootSignatureDesc, 0, sizeof(rootSignatureDesc));
    rootSignatureDesc.NumParameters = countof(param);
    rootSignatureDesc.pParameters = param;
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


    ComPtr<ID3DBlob> blob = nullptr;
    BRWL_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, NULL)), BRWL_CHAR_LITERAL("Failed to serialize root signature."));

    if (!BRWL_VERIFY(SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))), BRWL_CHAR_LITERAL("Failed to create root signature")))
    {
        destroy();
        return;
    }

    rootSignature->SetName(L"Initialization Root Signature");

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
    memset(&psoDesc, 0, sizeof(psoDesc));
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.CS = { Initialization_cs_cs, sizeof(Initialization_cs_cs) };
    psoDesc.NodeMask = 0;
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


    if (!BRWL_VERIFY(SUCCEEDED(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))), BRWL_CHAR_LITERAL("Failed to create pipeline state.")))
    {
        destroy();
        return;
    }

}

void InitializationShader::destroy()
{
    rootSignature = nullptr;
    pipelineState = nullptr;
}

InitializationShader::~InitializationShader()
{
    destroy();
}

void InitializationShader::draw(ID3D12GraphicsCommandList* cmd, const ShaderConstants& constants, ComputeBuffers* computeBuffers)
{
    SCOPED_GPU_EVENT(cmd, 0, 0, 0, "Initialization Compute Shader");
    BRWL_EXCEPTION(computeBuffers, nullptr);
    BRWL_EXCEPTION(computeBuffers->isResident(), nullptr);

    computeBuffers->beforeComputeUse(cmd);

    cmd->SetPipelineState(pipelineState.Get());
    cmd->SetComputeRootSignature(rootSignature.Get());
    cmd->SetComputeRoot32BitConstants(0, ShaderConstants::num32BitValues, &constants, 0);
    cmd->SetComputeRootDescriptorTable(1, computeBuffers->getTargetResourceDescriptorHandle(0).residentGpu);
   cmd->Dispatch(
        (unsigned int)std::ceil(computeBuffers->getWidth() / (float)ShaderConstants::threadGroupSizeX),
        (unsigned int)std::ceil(computeBuffers->getHeight() / (float)ShaderConstants::threadGroupSizeY),
        1
    );
    
   computeBuffers->afterComputeUse(cmd);
}

BRWL_RENDERER_NS_END