#include "PropagationShader.h"

#include "Propagation_cs_cs.h"

#include "ComputeBuffers.h"
#include "DxHelpers.h"
#include "TextureResource.h"
#include "PitCollection.h"


BRWL_RENDERER_NS


PropagationShader::PropagationShader(ID3D12Device* device) :
    rootSignature(nullptr),
    pipelineState(nullptr)
{

    // building pipeline state object and rootsignature

    {
        // ping-pong buffers - write
        D3D12_DESCRIPTOR_RANGE pingPongRanges[2];
        D3D12_DESCRIPTOR_RANGE& uavRange = pingPongRanges[0];
        uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        uavRange.NumDescriptors = 6;
        uavRange.BaseShaderRegister = 0;
        uavRange.RegisterSpace = 0;
        uavRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        // ping - pong buffers - read
        D3D12_DESCRIPTOR_RANGE& srvRange = pingPongRanges[1];
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 6;
        srvRange.BaseShaderRegister = 0;
        srvRange.RegisterSpace = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER param[8];
        memset(&param, 0, sizeof(param));
        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.Num32BitValues = 1;
        param[0].Constants.ShaderRegister = 0;
        param[0].Constants.RegisterSpace = 0;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges = &uavRange;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[2].DescriptorTable.NumDescriptorRanges = 1;
        param[2].DescriptorTable.pDescriptorRanges = &srvRange;
        param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // preintegration tables and volume texture
        D3D12_DESCRIPTOR_RANGE descriptorRanges[ENUM_CLASS_TO_NUM(PitTex::MAX) + 1];
        memset(&descriptorRanges, 0, sizeof(descriptorRanges));
        for (int i = 0; i < countof(descriptorRanges); ++i)
        {
            D3D12_DESCRIPTOR_RANGE& range = descriptorRanges[i];
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = 1;
            range.BaseShaderRegister = 6 + i;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }

        for (int i = 0; i < countof(descriptorRanges); ++i)
        {
            param[3 + i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param[3 + i].DescriptorTable.NumDescriptorRanges = 1;
            param[3 + i].DescriptorTable.pDescriptorRanges = &descriptorRanges[i];
            param[3 + i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }

        // volume and preintergration sampler are static samplers since they never change
        D3D12_STATIC_SAMPLER_DESC staticSamplers[2];
        makeStaticSamplerDescription(staticSamplers[0], 0);
        makeStaticSamplerDescription(staticSamplers[1], 1);

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        memset(&rootSignatureDesc, 0, sizeof(rootSignatureDesc));
        rootSignatureDesc.NumParameters = countof(param);
        rootSignatureDesc.pParameters = param;
        rootSignatureDesc.NumStaticSamplers = countof(staticSamplers);
        rootSignatureDesc.pStaticSamplers = staticSamplers;
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

        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc;
        memset(&psoDesc, 0, sizeof(psoDesc));
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.CS = { Propagation_cs_cs, sizeof(Propagation_cs_cs) };
        psoDesc.NodeMask = 0;
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;


        if (!BRWL_VERIFY(SUCCEEDED(device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))), BRWL_CHAR_LITERAL("Failed to create pipeline state.")))
        {
            destroy();
            return;
        }
    }
}

void PropagationShader::destroy()
{
    rootSignature = nullptr;
    pipelineState = nullptr;
}

PropagationShader::~PropagationShader()
{
    destroy();
}

void PropagationShader::draw(ID3D12GraphicsCommandList* cmd, const PropagationShader::DrawData& data,
    ComputeBuffers* computeBuffers, const PitCollection* pitCollection, const TextureResource* volumeTexture)
{
    SCOPED_GPU_EVENT(cmd, 0, 255, 0, "Propagation Compute Shader");
    BRWL_EXCEPTION(computeBuffers, nullptr);
    BRWL_EXCEPTION(computeBuffers->isResident(), nullptr);

    

    cmd->SetPipelineState(pipelineState.Get());
    cmd->SetComputeRootSignature(rootSignature.Get());

    // constants
    cmd->SetComputeRoot32BitConstants(0, 1, &data.sliceWidth, 0);

    // Set preintegration tables
    for (int i = 0; i < ENUM_CLASS_TO_NUM(PitTex::MAX); ++i)
    {
        cmd->SetComputeRootDescriptorTable(3 + i, pitCollection->array[i].liveTexture->descriptorHandle->getGpu());
    }

    // set volume texture
    cmd->SetComputeRootDescriptorTable(3 + ENUM_CLASS_TO_NUM(PitTex::MAX), volumeTexture->descriptorHandle->getGpu());

    float actualResX = Utils::min<float>(data.textureResolution.x, computeBuffers->getWidth());
    float actualResY = Utils::min<float>(data.textureResolution.y, computeBuffers->getHeight());


    int i = 0;
    do {
        computeBuffers->swap(cmd);
        // cmd->SetComputeRoot32BitConstants(0, ShaderConstants::num32BitValues, &constants, 0);
        cmd->SetComputeRootDescriptorTable(1, computeBuffers->getTargetUav(0).residentGpu);
        cmd->SetComputeRootDescriptorTable(2, computeBuffers->getSourceSrv(0).residentGpu);

        cmd->Dispatch(
            (unsigned int)std::ceil(actualResX / (float)DrawData::threadGroupSizeX),
            (unsigned int)std::ceil(actualResY / (float)DrawData::threadGroupSizeY),
            1
        );

    } while (++i < data.numSlices);

}

BRWL_RENDERER_NS_END