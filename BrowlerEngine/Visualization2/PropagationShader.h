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
        static const unsigned int threadGroupSizeX = 16;
        static const unsigned int threadGroupSizeY = 16;
        Vec3 bboxmin;
        float padding0;
        Vec3 bboxmax;
        float sliceWidth;
        static const unsigned int constantCount = 8;
    };
#pragma pack(pop)
    // propagates light and viewing rays through the volume and returns the amount of slices which was not yet able to process, returns also the last written 
    // color buffer via outColorBufferResource and outColorBufferDescriptorHandle
    unsigned int draw(ID3D12GraphicsCommandList* cmd, const DrawData& data, ComputeBuffers* computeBuffers, 
        PitCollection& pitCollection, TextureHandle& volumeTexture, unsigned int remainingSlices);

private:
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    int slicesPerInvocation;
};

BRWL_RENDERER_NS_END