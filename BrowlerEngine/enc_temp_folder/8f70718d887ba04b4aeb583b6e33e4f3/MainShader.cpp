#include "MainShader.h"

#include "VertexShader_vs_vs.h"
#include "PixelShader_ps_ps.h"

#include "Renderer/PAL/d3dx12.h"

#include "Common/ProceduralGeometry.h"
#include "Core/BrowlerEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"
#include "Core/Input.h"

#include "TextureResource.h"
#include "PitCollection.h"
#include "Common/BoundingBox.h"


namespace
{
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

    const int mvpIdx = 0;
    const int voxelsPerCmIdx = mvpIdx + 1;

    const int texturesMin = voxelsPerCmIdx + 1;
    const int refractionIntegTexIdx = texturesMin;
    const int particleColIntegTexIdx = refractionIntegTexIdx + 1;
    const int opacityIntegTexIdx = particleColIntegTexIdx + 1;
    const int mediumIntegTexIdx = opacityIntegTexIdx + 1;
    const int volumeTextureIdx = mediumIntegTexIdx + 1;
    const int numTextures = volumeTextureIdx + 1 - texturesMin;
    const int numRootParameters = volumeTextureIdx + 1;

}


BRWL_RENDERER_NS


MainShader::MainShader() :
    rootSignature(nullptr),
    pipelineState(nullptr),
    initialized(false),
    viewingPlane{}
{ }

bool MainShader::create(ID3D12Device* device)
{
    if (initialized) return true;

#pragma region PSO
    {
        // volume and preintergration sampler are static samplers since they never change
        unsigned int samplerRegister = 0;
        D3D12_STATIC_SAMPLER_DESC staticSamplers[2];
        D3D12_STATIC_SAMPLER_DESC& preintegrationSampler = staticSamplers[0];
        makeStaticSamplerDescription(preintegrationSampler, samplerRegister++);
        D3D12_STATIC_SAMPLER_DESC& volumeSampler = staticSamplers[1];
        makeStaticSamplerDescription(volumeSampler, samplerRegister++);

        // textures go each into a single table of srv descriptor ranges
        // each table and each range are of size 1 because the handles are not located in a continuous range
        D3D12_DESCRIPTOR_RANGE srvDescRanges[numTextures];
        memset(&srvDescRanges, 0, sizeof(srvDescRanges));
        for (int i = 0; i < numTextures; ++i)
        {
            D3D12_DESCRIPTOR_RANGE& srvRange = srvDescRanges[i];
            srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            srvRange.NumDescriptors = 1;
            srvRange.BaseShaderRegister = i; // starting with register(t0), going up to register(t[numTextures])
            srvRange.RegisterSpace = 0;
            srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }

        unsigned int cbufferRegister = 0;
        D3D12_ROOT_PARAMETER param[numRootParameters];
        memset(&param, 0, sizeof(param));
        // Model- & ViewProjection matrix & voxelsPerCm
        param[mvpIdx].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[mvpIdx].Constants.ShaderRegister = cbufferRegister++;
        param[mvpIdx].Constants.RegisterSpace = 0;
        param[mvpIdx].Constants.Num32BitValues = 33;
        param[mvpIdx].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        // voxelsPerCm
        param[voxelsPerCmIdx].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[voxelsPerCmIdx].Constants.ShaderRegister = cbufferRegister++;
        param[voxelsPerCmIdx].Constants.RegisterSpace = 0;
        param[voxelsPerCmIdx].Constants.Num32BitValues = 5;
        param[voxelsPerCmIdx].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        // Root srv parameters for textures (because the descriptor handles are not located in a continuous range)
        for (int i = texturesMin; i < texturesMin + numTextures; ++i)
        {
            param[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param[i].DescriptorTable.NumDescriptorRanges = 1;
            param[i].DescriptorTable.pDescriptorRanges = &srvDescRanges[i - texturesMin]; 
            param[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        }

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        memset(&rootSignatureDesc, 0, sizeof(rootSignatureDesc));
        rootSignatureDesc.NumParameters = countof(param);
        rootSignatureDesc.pParameters = param;
        rootSignatureDesc.NumStaticSamplers = countof(staticSamplers);
        rootSignatureDesc.pStaticSamplers = staticSamplers;
        rootSignatureDesc.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


        ComPtr<ID3DBlob> blob = nullptr;
        if (!BRWL_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, NULL)), BRWL_CHAR_LITERAL("Failed to serialize root signature.")))
        {
            destroy();
            return false;
        }

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))), BRWL_CHAR_LITERAL("Failed to create root signature")))
        {
            destroy();
            return false;
        }
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
    psoDesc.VS = { VertexShader_vs_vs, sizeof(VertexShader_vs_vs) };
    psoDesc.PS = { PixelShader_ps_ps, sizeof(PixelShader_ps_ps) };

    // Create the input layout
    static D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, (UINT)offsetof(VertexData, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)offsetof(VertexData, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, (UINT)offsetof(VertexData, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    psoDesc.InputLayout = { inputLayout, (unsigned int)countof(inputLayout) };


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

    if (!BRWL_VERIFY(SUCCEEDED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState))), BRWL_CHAR_LITERAL("Failed to create pipeline state.")))
    {
        destroy();
        return false;
    }
#pragma endregion
    {
        std::vector<VertexData> tris;
        ProceduralGeometry::makeQuad(1, 1, tris);
        viewingPlane.vertexBufferLength = (unsigned int)tris.size();

        BRWL_CHECK(viewingPlane.vertexBuffer == nullptr, nullptr);
        D3D12_HEAP_PROPERTIES heapProperties;
        memset(&heapProperties, 0, sizeof(D3D12_HEAP_PROPERTIES));
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = tris.size() * sizeof(VertexData);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommittedResource(
            &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&viewingPlane.vertexBuffer))), nullptr))
        {
            destroy();
            return false;
        }

        viewingPlane.vertexBuffer->SetName(L"Viewing plane vertex buffer");

        VertexData* vtx_resource;
        D3D12_RANGE range{ 0 , 0 };
        if (!BRWL_VERIFY(SUCCEEDED(viewingPlane.vertexBuffer->Map(0, &range, (void**)&vtx_resource)), nullptr))
        {
            destroy();
            return false;
        }

        memcpy(vtx_resource, tris.data(), tris.size() * sizeof(decltype(tris)::value_type));

        viewingPlane.vertexBuffer->Unmap(0, nullptr); // nullptr means that we may have written the entire range

    }

    // TODO: do we need some synchonisation here?

    initialized = true;
    return initialized;
}

void MainShader::render()
{
    if (!initialized) return;
    //if (vertexBuffer == nullptr)
}

void MainShader::draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmd, const DrawData& data)
{
    setup(cmd);


    // Setup viewport
    D3D12_VIEWPORT vp;
    memset(&vp, 0, sizeof(vp));
    unsigned int width, height;
    engine->renderer->getFrameBufferSize(width, height);
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.TopLeftX = vp.TopLeftY = 0.0f;
    cmd->RSSetViewports(1, &vp);

    // scissor
    const D3D12_RECT r = { 0, 0, width, height };
    cmd->RSSetScissorRects(1, &r);

    BRWL::RENDERER::Camera* cam = engine->renderer->getCamera();
    // mvp matrix
    Mat4 viewProjection;
    if (cam != nullptr) {
        vp.MinDepth = cam->getNearPlane();
        vp.MaxDepth = cam->getFarPlane();
        viewProjection = cam->getViewProjectionMatrix();
    }
    else
    {
        vp.MinDepth = 0.f;
        vp.MaxDepth = 10.f;
        viewProjection = identity();
    }

    // position viewing plane
    const Quaternion camRotation = cam->getGlobalOrientation();
    BBox viewAlignedBBoxContainingRotatedOBB = data.volumeDimensions->getOBB(camRotation);
    Vec3 quadScale(viewAlignedBBoxContainingRotatedOBB.dimX(), viewAlignedBBoxContainingRotatedOBB.dimY(), viewAlignedBBoxContainingRotatedOBB.dimZ());
    const Vec3 camPos = cam->getGlobalPosition();
    // the nearest plane
    const float planeOffsetNear = data.volumeDimensions->getClosestPlaneFromDirection(camPos);
    // the farthes plane
    const float planeOffsetFar = data.volumeDimensions->getClosestPlaneFromDirection(-camPos);
    const float planeStackThickness = std::abs(planeOffsetFar - -planeOffsetNear);
    const float numSlices = planeStackThickness; // 1 plane per voxel is a very good resolution
    const Vec3 deltaSlice = normalized(-camPos) * planeStackThickness / numSlices ;
    Mat4 modelMatrix = makeRotationMatrix(Vec4(camRotation.x, camRotation.y, camRotation.z, camRotation.w)) * makeAffineTransform(
        normalized(-camPos) * (planeOffsetNear / data.voxelsPerCm),
        Quaternion(),
        quadScale / data.voxelsPerCm
    );

    cmd->SetGraphicsRoot32BitConstants(mvpIdx, 16, &modelMatrix, 0);
    cmd->SetGraphicsRoot32BitConstants(mvpIdx, 16, &viewProjection, 16);
    cmd->SetGraphicsRoot32BitConstants(mvpIdx, 1, &data.voxelsPerCm, 32);
    cmd->SetGraphicsRoot32BitConstants(voxelsPerCmIdx, 1, &data.voxelsPerCm, 0);
    cmd->SetGraphicsRoot32BitConstants(voxelsPerCmIdx, 1, &numSlices, 1);
    cmd->SetGraphicsRoot32BitConstants(voxelsPerCmIdx, 3, &deltaSlice, 2);
    // Set all textures
    // the volume texture is the one with the hightest register index by convention, so we leave it out in the loop
    for (int i = texturesMin; i < texturesMin + numTextures - 1; ++i)
    {
        cmd->SetGraphicsRootDescriptorTable(i, data.pitCollection->array[i - texturesMin].liveTexture->descriptorHandle.gpu);
    }

    cmd->SetGraphicsRootDescriptorTable(volumeTextureIdx, data.volumeTexture->descriptorHandle.gpu);

    cmd->DrawInstanced(viewingPlane.vertexBufferLength, 1, 0, 0);

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    //int global_vtx_offset = 0;
    //int global_idx_offset = 0;
    //ImVec2 clip_off = draw_data->DisplayPos;
    //for (int n = 0; n < draw_data->CmdListsCount; n++)
    //{
        //const ImDrawList* cmd_list = draw_data->CmdLists[n];
        //for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        //{
        //    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
        //    if (pcmd->UserCallback != NULL)
        //    {
        //        // User callback, registered via ImDrawList::AddCallback()
        //        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
        //        if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
        //            ImGui_ImplDX12_SetupRenderState(draw_data, ctx, fr);
        //        else
        //            pcmd->UserCallback(cmd_list, pcmd);
        //    }
        //    else
        //    {
        //        if (pcmd->ClipRect.z <= 0 || pcmd->ClipRect.x >= draw_data->DisplaySize.x || pcmd->ClipRect.w <= 0 || pcmd->ClipRect.y >= draw_data->DisplaySize.y ||
        //            pcmd->ClipRect.x == pcmd->ClipRect.z || pcmd->ClipRect.y == pcmd->ClipRect.w)
        //            continue;
                // Apply Scissor, Bind texture, Draw
                //const D3D12_RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
                //cmd->SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)&pcmd->TextureId);
                //ctx->RSSetScissorRects(1, &r);
                //cmd->DrawIndexedInstanced(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
        //    }
        //}
        //global_idx_offset += cmd_list->IdxBuffer.Size;
        //global_vtx_offset += cmd_list->VtxBuffer.Size;  
    //}





}

void MainShader::destroy()
{
    if (!initialized) return;

    pipelineState = nullptr;
    rootSignature = nullptr;

    viewingPlane.destroy();

    initialized = false;
}

void MainShader::setup(ID3D12GraphicsCommandList* cmd)
{

    // Bind shader and vertex buffers
    unsigned int stride = sizeof(VertexData);
    unsigned int offset = 0;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    memset(&vbv, 0, sizeof(vbv));
    vbv.BufferLocation =  viewingPlane.vertexBuffer->GetGPUVirtualAddress() + offset;
    vbv.SizeInBytes = viewingPlane.vertexBufferLength * stride;
    vbv.StrideInBytes = stride;
    cmd->IASetVertexBuffers(0, 1, &vbv);
    /*D3D12_INDEX_BUFFER_VIEW ibv;
    memset(&ibv, 0, sizeof(ibv));
    ibv.BufferLocation = fr->IndexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes = fr->IndexBufferSize * sizeof(VertexData);
    ibv.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;*/
    //ctx->IASetIndexBuffer(&ibv);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmd->SetPipelineState(pipelineState.Get());
    cmd->SetGraphicsRootSignature(rootSignature.Get());

    // Setup blend factor
   /* const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    cmd->OMSetBlendFactor(blend_factor);*/

    //   // TODO: remove
//   
//   Vec3 worldRay = screenSpaceToWorldRay(engine->input->getMouseX(), engine->input->getMouseY(), *engine->renderer->getCamera());
//   Vec3 pos = engine->renderer->getCamera()->getGlobalPosition();
//   if (worldRay.y != 0 && std::abs(pos.y) - 0.02 > std::abs(worldRay.y + pos.y))
//   {
//       // intersect with plane y = 0;
//       worldRay *= pos.y / worldRay.y;
//       mvp = makeAffineTransform(Vec3(-worldRay.x, 0.5f, worldRay.z), { 90.f * DEG_2_RAD_F , 0.f, 0.f }, { 1,1,1 }) * viewProjection;

//   }
}

BRWL_RENDERER_NS_END