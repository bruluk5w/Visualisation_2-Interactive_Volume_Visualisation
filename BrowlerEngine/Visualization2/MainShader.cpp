#include "MainShader.h"

#include "OrthographicXRay_vs_vs.h"
#include "OrthographicXRay_ps_ps.h"
#include "Propagation_cs_cs.h"
#include "Guides_vs_vs.h"
#include "Guides_ps_ps.h"

#include "Renderer/PAL/d3dx12.h"

#include "Common/ProceduralGeometry.h"
#include "Core/BrowlerEngine.h"
#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"
#include "Core/Input.h"

#include "TextureResource.h"
#include "PitCollection.h"
#include "Common/BoundingBox.h"
#include "Common/Logger.h"
#include "ComputeBuffers.h"
#include "InitializationShader.h"
#include "PropagationShader.h"
#include "DxHelpers.h"
#include "ImposterShader.h"
#include "Common/PAL/DescriptorHeap.h"

namespace
{
    void transitionResourceFromPixelToComputeShader(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource)
    {
        D3D12_RESOURCE_BARRIER barrier;
        memset(&barrier, 0, sizeof(barrier));
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = 0;
        cmd->ResourceBarrier(1, &barrier);
    }

    void transitionResourceFromComputeToPixelShader(ID3D12GraphicsCommandList* cmd, ID3D12Resource* resource)
    {
        D3D12_RESOURCE_BARRIER barrier;
        memset(&barrier, 0, sizeof(barrier));
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = 0;
        cmd->ResourceBarrier(1, &barrier);
    }

    // Input Layout is the same for all shaders
    static const D3D12_INPUT_ELEMENT_DESC inputElements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, (UINT)offsetof(BRWL::VertexData, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)offsetof(BRWL::VertexData, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    static const D3D12_INPUT_LAYOUT_DESC inputLayout = { inputElements, BRWL::countof(inputElements) };

}


BRWL_RENDERER_NS

MainShader::TriangleList::TriangleList() :
    vertexBuffer(nullptr),
    vertexBufferLength(0),
    vertexBufferView{},
    loaded(false)
{ }

bool MainShader::TriangleList::load(ID3D12Device* device, const VertexData* vertices, unsigned int numVertices)
{
    BRWL_CHECK(vertices != nullptr, nullptr);
    BRWL_CHECK(!loaded, BRWL_CHAR_LITERAL("Invalid triangle list state."));
    BRWL_CHECK(vertexBuffer == nullptr, BRWL_CHAR_LITERAL("Invalid triangle list state."));
    BRWL_EXCEPTION(numVertices > 0, BRWL_CHAR_LITERAL("Loading empty triangle list"));

    loaded = false;

    vertexBufferLength = numVertices;

    D3D12_HEAP_PROPERTIES heapProperties;
    memset(&heapProperties, 0, sizeof(D3D12_HEAP_PROPERTIES));
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = numVertices * sizeof(VertexData);
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    if (!BRWL_VERIFY(SUCCEEDED(device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&vertexBuffer))), nullptr))
    {
        destroy();
        return false;
    }


    VertexData* vtx_resource;
    D3D12_RANGE range{ 0 , 0 };
    if (!BRWL_VERIFY(SUCCEEDED(vertexBuffer->Map(0, &range, (void**)&vtx_resource)), nullptr))
    {
        destroy();
        return false;
    }

    memcpy(vtx_resource, vertices, numVertices * sizeof(VertexData));

    vertexBuffer->Unmap(0, nullptr); // nullptr means that we may have written the entire range
    loaded = true;
    return true;
}

void MainShader::TriangleList::destroy()
{
    loaded = false;
    vertexBuffer = nullptr;
    vertexBufferLength = 0;
    memset(&vertexBufferView, 0, sizeof(vertexBufferView));
}


MainShader::MainShader() :
    mainRootSignature(nullptr),
    mainPipelineState(nullptr),
    guidesRootSignature(nullptr),
    guidesPipelineState(nullptr),
    initialized(false),
    viewingPlane{},
    initializationShader(nullptr),
    propagationShader(nullptr),
    computeBuffers(nullptr),
    imposterShader(nullptr)
{
    computeBuffers = std::make_unique<ComputeBuffers>();
}

MainShader::~MainShader()
{
    computeBuffers = nullptr;
}

bool MainShader::create(Renderer* renderer)
{
    if (initialized) return true;

    ID3D12Device* device = renderer->device.Get();

#pragma region Main PSO
    {
        D3D12_ROOT_PARAMETER param[7];
        memset(&param, 0, sizeof(param));

        // Vertex shader constants
        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.ShaderRegister = 0;
        param[0].Constants.RegisterSpace = 0;
        param[0].Constants.Num32BitValues = 33;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // Pixel shader constants
        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[1].Constants.ShaderRegister = 1;
        param[1].Constants.RegisterSpace = 0;
        param[1].Constants.Num32BitValues = 7;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        
        // preintegration tables and volume texture
        D3D12_DESCRIPTOR_RANGE descriptorRanges[ENUM_CLASS_TO_NUM(PitTex::MAX) + 1];
        memset(&descriptorRanges, 0, sizeof(descriptorRanges));
        for (int i = 0; i < countof(descriptorRanges); ++i)
        {
            D3D12_DESCRIPTOR_RANGE& range = descriptorRanges[i];
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.NumDescriptors = 1;
            range.BaseShaderRegister = i;
            range.RegisterSpace = 0;
            range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }
        
        for (int i = 0; i < countof(descriptorRanges); ++i)
        {
            param[2 + i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            param[2 + i].DescriptorTable.NumDescriptorRanges = 1;
            param[2 + i].DescriptorTable.pDescriptorRanges = &descriptorRanges[i];
            param[2 + i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
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
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


        ComPtr<ID3DBlob> blob = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;
        if (!BRWL_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, &errorBlob)), BRWL_CHAR_LITERAL("Failed to serialize root signature.")))
        {
            if (errorBlob != nullptr) {
                engine->logger->error((BRWL_CHAR*)errorBlob->GetBufferPointer());
            }
            destroy();
            return false;
        }

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&mainRootSignature))), BRWL_CHAR_LITERAL("Failed to create root signature")))
        {
            destroy();
            return false;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        memset(&psoDesc, 0, sizeof(psoDesc));
        psoDesc.NodeMask = 0;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.pRootSignature = mainRootSignature.Get();
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.NumRenderTargets = 3;
        psoDesc.RTVFormats[0] = PAL::g_RenderTargetFormat;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        psoDesc.VS = { OrthographicXRay_vs_vs, sizeof(OrthographicXRay_vs_vs) };
        psoDesc.PS = { OrthographicXRay_ps_ps, sizeof(OrthographicXRay_ps_ps) };
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

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mainPipelineState))), BRWL_CHAR_LITERAL("Failed to create pipeline state.")))
        {
            destroy();
            return false;
        }
    }
#pragma endregion Main PSO
#pragma region Guides PSO
    {
        unsigned int cbufferRegister = 0;
        D3D12_ROOT_PARAMETER param[1];
        memset(&param, 0, sizeof(param));
        // ModelViewProjection matrix
        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        param[0].Constants.ShaderRegister = cbufferRegister++;
        param[0].Constants.RegisterSpace = 0;
        param[0].Constants.Num32BitValues = 16;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        memset(&rootSignatureDesc, 0, sizeof(rootSignatureDesc));
        rootSignatureDesc.NumParameters = countof(param);
        rootSignatureDesc.pParameters = param;
        rootSignatureDesc.Flags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        ComPtr<ID3DBlob> blob = nullptr;
        if (!BRWL_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &blob, NULL)), BRWL_CHAR_LITERAL("Failed to serialize root signature.")))
        {
            destroy();
            return false;
        }

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&guidesRootSignature))), BRWL_CHAR_LITERAL("Failed to create root signature")))
        {
            destroy();
            return false;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
        memset(&psoDesc, 0, sizeof(psoDesc));
        psoDesc.NodeMask = 0;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.pRootSignature = guidesRootSignature.Get();
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.NumRenderTargets = 3;
        psoDesc.RTVFormats[0] = PAL::g_RenderTargetFormat;
        psoDesc.SampleDesc.Count = 1;
        psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        psoDesc.VS = { Guides_vs_vs, sizeof(Guides_vs_vs) };
        psoDesc.PS = { Guides_ps_ps, sizeof(Guides_ps_ps) };
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

        if (!BRWL_VERIFY(SUCCEEDED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&guidesPipelineState))), BRWL_CHAR_LITERAL("Failed to create pipeline state.")))
        {
            destroy();
            return false;
        }
    }
#pragma endregion

    {
        std::vector<VertexData> tris;
        ProceduralGeometry::makeQuad(1, 1, tris);
        if (!BRWL_VERIFY(viewingPlane.load(device, tris.data(), tris.size()), BRWL_CHAR_LITERAL("Failed to load viewing plane geometry.")))
        {
            destroy();
            return false;
        }

        viewingPlane.vertexBuffer->SetName(L"Viewing plane vertex buffer");

        ProceduralGeometry::makeCube(1, 1, 1, tris);
        if (!BRWL_VERIFY(assetBounds.load(device, tris.data(), tris.size()), BRWL_CHAR_LITERAL("Failed to load viewing plane geometry.")))
        {
            destroy();
            return false;
        }
        
        assetBounds.vertexBuffer->SetName(L"Asset bounds vertex buffer");
    }

    // Create compute buffers
    if (!computeBuffers->create(device, &renderer->getSrvHeap(), DrawData::gatherTextureSize, DrawData::gatherTextureSize))
    {
        destroy();
        return false;
    }

    // TODO: do we need some synchonisation here?

    initializationShader = std::make_unique<InitializationShader>(device);
    propagationShader = std::make_unique <PropagationShader>(device);
    imposterShader = std::make_unique <ImposterShader>(device, inputLayout);

    initialized = true;
    return initialized;
}

void MainShader::render()
{
    if (!initialized) return;
    //if (vertexBuffer == nullptr)
}

void MainShader::draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmd, const MainShader::DrawData& data)
{
    SCOPED_GPU_EVENT(cmd, 0, 128, 128, "Main Draw");
    unsigned int width, height;
    engine->renderer->getFrameBufferSize(width, height);

    BRWL::RENDERER::Camera* cam = engine->renderer->getCamera();
    BRWL_EXCEPTION(cam != nullptr, nullptr);
    const Quaternion camRot = cam->getGlobalOrientation();
    const Vec3 camPos = cam->getGlobalPosition();
    const Mat4& viewProjection = cam->getViewProjectionMatrix();

    // Setup viewport
    D3D12_VIEWPORT wholeScreenVp;
    memset(&wholeScreenVp, 0, sizeof(wholeScreenVp));
    wholeScreenVp.MinDepth = cam->getNearPlane();
    wholeScreenVp.MaxDepth = cam->getFarPlane();
    wholeScreenVp.Width = (float)width;
    wholeScreenVp.Height = (float)height;
    wholeScreenVp.TopLeftX = wholeScreenVp.TopLeftY = 0.0f;
    cmd->RSSetViewports(1, &wholeScreenVp);

    const float volumeScale = 1.f / data.voxelsPerCm;

    // positioning of the viewing plane
    const Mat4 viewingVolumeOrientation = inverse(makeLookAtTransform(VEC3_ZERO, -camPos));
    const BBox viewingVolumeUnscaled = data.volumeDimensions->getOBB(viewingVolumeOrientation);
    const Vec3 viewingVolumeDimensions = viewingVolumeUnscaled.dim() * volumeScale;

    const Mat4 viewingPlaneModelMatrix =
        makeAffineTransform({ 0, 0, -0.5f * viewingVolumeDimensions.z }, VEC3_ZERO, viewingVolumeDimensions) *
        makeAffineTransform({ 0, 0, 0 }, VEC3_ZERO, VEC3_ONE) * viewingVolumeOrientation;

    const Vec2 viewingPlaneDimensions(viewingVolumeDimensions.x, viewingVolumeDimensions.y);
    const Vec2 viewingPlaneDimensionsUnscaled(viewingVolumeUnscaled.dimX(), viewingVolumeUnscaled.dimY());

    // the nearest plane in texture space
    const float planeOffsetNear = data.volumeDimensions->getClosestPlaneFromDirection(camPos);
    // the farthes plane in texture space
    const float planeOffsetFar = -data.volumeDimensions->getClosestPlaneFromDirection(-camPos);
    const float planeStackThickness = planeOffsetNear - planeOffsetFar;
    BRWL_CHECK(planeStackThickness > 0, nullptr);
    const float numSlices = planeStackThickness; // 1 plane per voxel is a very good resolution
    // position offset from prevous to next plane
    const float sliceWidth = planeStackThickness / (numSlices  * data.voxelsPerCm);
    const Vec3 deltaSlice = normalized(-camPos) * sliceWidth;

    if (engine->input->isKeyDown(Key::F1))
    {
        const Vec3 fwd = camRot.forward();
        BRWL_CHAR buf[100];
        BRWL_SNPRINTF(buf, countof(buf),
            BRWL_CHAR_LITERAL("Camera Transform: Position:%.2f | %.2f | %.2f, Look Direction: %.2f | %.2f | %.2f"),
            camPos.x, camPos.y, camPos.z, fwd.x, fwd.y, fwd.z);
        engine->logger->info(buf);
    }

    if (engine->input->isKeyDown(Key::F2))
    {
        BRWL_CHAR buf[100];
        BRWL_SNPRINTF(buf, countof(buf), BRWL_CHAR_LITERAL("%.2f | %.2f | %.2f"), deltaSlice.x, deltaSlice.y, deltaSlice.z);
        engine->logger->info(buf);
    }

    bool hasComputeBuffers = computeBuffers->isResident();
    if (hasComputeBuffers && !data.drawOrthographicXRay)
    {
        // Dispatch ping-pong compute work 
        InitializationShader::ShaderConstants initParams;
        {
            const Vec3 viewingPlaneCenter = extractPosition(viewingPlaneModelMatrix);
            const Vec3 right = normalized(toVec3(VEC4_RIGHT * viewingPlaneModelMatrix));
            const Vec3 down = normalized(toVec3(-VEC4_UP * viewingPlaneModelMatrix));
            const Vec2 halfDimension = (0.5f * viewingPlaneDimensions);

            initParams.textureSizeWorldSpace = viewingPlaneDimensions;
            initParams.textureResolution = viewingPlaneDimensionsUnscaled;
            initParams.horizontalPlaneDirection = right;
            initParams.verticalPlaneDirection = down;
            initParams.topLeft = viewingPlaneCenter - halfDimension.x * right - halfDimension.y * down; // in world space
            initParams.eye = camPos;
            initParams.lightDirection = Quaternion::fromTo(VEC3_FWD, -camPos) * data.light.coords; // relative to eye-origin vector, so that the light source stays in the same hemisphere when we move around then origin
            initParams.lightColor = data.light.color;
        }

        initializationShader->draw(cmd, initParams, computeBuffers.get());

        PropagationShader::DrawData propParams;
        {
            propParams.textureResolution = viewingPlaneDimensionsUnscaled;
            propParams.numSlices = numSlices;
            propParams.sliceWidth = sliceWidth;
            propParams.voxelsPerCm = data.voxelsPerCm;
            propParams.volumeTexelDimensions = data.volumeDimensions->dim();
        }

        computeBuffers->swap(cmd);

        transitionResourceFromPixelToComputeShader(cmd, data.volumeTexture->texture.Get());
        transitionResourceFromPixelToComputeShader(cmd, data.pitCollection->tables.mediumColorPit.liveTexture->texture.Get());
        transitionResourceFromPixelToComputeShader(cmd, data.pitCollection->tables.opacityPit.liveTexture->texture.Get());
        transitionResourceFromPixelToComputeShader(cmd, data.pitCollection->tables.particleColorPit.liveTexture->texture.Get());
        transitionResourceFromPixelToComputeShader(cmd, data.pitCollection->tables.refractionPit.liveTexture->texture.Get());

        ID3D12Resource* colorBuffer;
        PAL::DescriptorHandle::NativeHandles colorBufferDescriptorHandle;
        propagationShader->draw(cmd, propParams, computeBuffers.get(), data.pitCollection, data.volumeTexture, colorBuffer, colorBufferDescriptorHandle);
        
        transitionResourceFromComputeToPixelShader(cmd, colorBuffer);
        // scissor
        const D3D12_RECT r = { 0, 0, width, height }; // whole frame buffer
        cmd->RSSetScissorRects(1, &r);

        bindVertexBuffer(cmd, viewingPlane);
        ImposterShader::VsConstants imposterVsConstants;
        memset(&imposterVsConstants, 0, sizeof(imposterVsConstants));
        {
            imposterVsConstants.modelviewProjection = viewingPlaneModelMatrix * viewProjection;
            // we have a scale and offset because of the buffer around the area where actual results are propagated
            imposterVsConstants.uvOffset.x = (float)PropagationShader::DrawData::bufferWidth / (float)computeBuffers->getWidth();
            imposterVsConstants.uvOffset.y = (float)PropagationShader::DrawData::bufferWidth / (float)computeBuffers->getHeight();
            const Vec2 actualRes(
                Utils::min<float>(viewingPlaneDimensionsUnscaled.x, (float)computeBuffers->getWidth() - 2.f * (float)PropagationShader::DrawData::bufferWidth),
                Utils::min<float>(viewingPlaneDimensionsUnscaled.y, (float)computeBuffers->getHeight() - 2.f * (float)PropagationShader::DrawData::bufferWidth));
            imposterVsConstants.uvRangeScale.x = actualRes.x / computeBuffers->getWidth();
            imposterVsConstants.uvRangeScale.y = actualRes.y / computeBuffers->getHeight();
        }

        imposterShader->setupDraw(cmd, imposterVsConstants, colorBufferDescriptorHandle);

        cmd->DrawInstanced(viewingPlane.vertexBufferLength, 1, 0, 0);

        transitionResourceFromComputeToPixelShader(cmd, data.volumeTexture->texture.Get());
        transitionResourceFromComputeToPixelShader(cmd, data.pitCollection->tables.mediumColorPit.liveTexture->texture.Get());
        transitionResourceFromComputeToPixelShader(cmd, data.pitCollection->tables.opacityPit.liveTexture->texture.Get());
        transitionResourceFromComputeToPixelShader(cmd, data.pitCollection->tables.particleColorPit.liveTexture->texture.Get());
        transitionResourceFromComputeToPixelShader(cmd, data.pitCollection->tables.refractionPit.liveTexture->texture.Get());

        transitionResourceFromPixelToComputeShader(cmd, colorBuffer);
    }

    // scissor
    const D3D12_RECT r = { 0, 0, width, height }; // whole frame buffer
    cmd->RSSetScissorRects(1, &r);
    
    // todo:draw viewing plane
    if (data.drawOrthographicXRay)
    {
       
        cmd->SetPipelineState(mainPipelineState.Get());
        cmd->SetGraphicsRootSignature(mainRootSignature.Get());

        bindVertexBuffer(cmd, viewingPlane);

        VsConstants vsConstants;
        memset(&vsConstants, 0, sizeof(vsConstants));
        vsConstants.modelMatrix = viewingPlaneModelMatrix;
        vsConstants.viewProjection = viewProjection;
        vsConstants.voxelsPerCm = data.voxelsPerCm;
        
        cmd->SetGraphicsRoot32BitConstants(0, 33, &vsConstants, 0);

        PsConstants psConstants;
        memset(&psConstants, 0, sizeof(psConstants));
        psConstants.voxelsPerCm = data.voxelsPerCm;
        psConstants.numSlices = numSlices;
        psConstants.deltaSlice = deltaSlice / sliceWidth;

        cmd->SetGraphicsRoot32BitConstants(1, 7, &psConstants, 0);

        // Set all textures
        for (int i = 0; i < ENUM_CLASS_TO_NUM(PitTex::MAX); ++i)
        {
            cmd->SetGraphicsRootDescriptorTable(2 + i, data.pitCollection->array[i].liveTexture->descriptorHandle->getGpu());
        }

        cmd->SetGraphicsRootDescriptorTable(2 + ENUM_CLASS_TO_NUM(PitTex::MAX), data.volumeTexture->descriptorHandle->getGpu());

        cmd->DrawInstanced(viewingPlane.vertexBufferLength, 1, 0, 0);
    }

    // draw asset bounds
    if (data.drawAssetBounds || data.drawViewingVolume)
    {
        bindVertexBuffer(cmd, assetBounds);
        cmd->SetPipelineState(guidesPipelineState.Get());
        cmd->SetGraphicsRootSignature(guidesRootSignature.Get());
        if (data.drawAssetBounds)
        {
            Mat4 mat = makeAffineTransform(VEC3_ZERO, Quaternion::identity, data.volumeDimensions->dim() * volumeScale);
            mat = mat * viewProjection;
            cmd->SetGraphicsRoot32BitConstants(0, 16, &mat, 0);
            cmd->DrawInstanced(assetBounds.vertexBufferLength, 1, 0, 0);
        }
        if (data.drawViewingVolume)
        {
            Mat4 mat = makeAffineTransform(VEC3_ZERO, VEC3_ZERO, viewingVolumeDimensions) * viewingVolumeOrientation * viewProjection;
            cmd->SetGraphicsRoot32BitConstants(0, 16, &mat, 0);
            cmd->DrawInstanced(assetBounds.vertexBufferLength, 1, 0, 0);
        }
    }
}

void MainShader::destroy()
{
    computeBuffers->destroy();
    propagationShader = nullptr;
    initializationShader = nullptr;
    assetBounds.destroy();
    viewingPlane.destroy();
    guidesPipelineState = nullptr;
    guidesRootSignature = nullptr;
    mainPipelineState = nullptr;
    mainRootSignature = nullptr;

    initialized = false;
}

void MainShader::bindVertexBuffer(ID3D12GraphicsCommandList* cmd, const TriangleList& list)
{
    // Bind shader and vertex buffers
    unsigned int stride = sizeof(VertexData);
    unsigned int offset = 0;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    memset(&vbv, 0, sizeof(vbv));
    vbv.BufferLocation = list.vertexBuffer->GetGPUVirtualAddress() + offset;
    vbv.SizeInBytes = list.vertexBufferLength * stride;
    vbv.StrideInBytes = stride;
    cmd->IASetVertexBuffers(0, 1, &vbv);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

BRWL_RENDERER_NS_END

