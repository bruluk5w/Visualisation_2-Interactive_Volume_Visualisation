#pragma once // (c) 2020 Lukas Brunner & Jonas Prohaska

#include "Renderer/PAL/DescriptorHeap.h"
#include "Renderer/TextureHandleFwd.h"

BRWL_RENDERER_NS

class ComputeBuffers;
union PitCollection;

class PropagationShader final
{
    void destroy();

public:
    PropagationShader(ID3D12Device* device);
    ~PropagationShader();

#pragma pack(push, 1)
    struct DrawData
    {
        static const unsigned int constantCount = 23;
        Vec3 bboxmin; // extents of the volume data set
        float texDimToUV; // 1 / compute buffer dimensions
        Vec3 bboxmax; // extents of the volume data set
        float worldDimToUV; // // 1 / world space width of texture
        Vec3 sliceNormal; // normal vector of the slices
        float texelDim; // size of a texel on the slice plane
        Vec3 planeRight; // horizontal direction of the slices in positive uv direction (normalised)
        float backgroundScale;
        Vec3 planeDown; // vertical direction of the slices in positive uv direction (normalised)
        float sliceDepth; // distance between two slices
        Vec3 topLeft; // top left corner point (front) of the viewing volume
    };
#pragma pack(pop)
    // propagates light and viewing rays through the volume and returns the amount of slices which was not yet able to process, returns also the last written 
    // color buffer via outColorBufferResource and outColorBufferDescriptorHandle
    unsigned int draw(ID3D12GraphicsCommandList* cmd, DrawData& data, ComputeBuffers* computeBuffers, 
        PitCollection& pitCollection, TextureHandle& volumeTexture, unsigned int remainingSlices);

private:
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    int slicesPerInvocation;
};

BRWL_RENDERER_NS_END