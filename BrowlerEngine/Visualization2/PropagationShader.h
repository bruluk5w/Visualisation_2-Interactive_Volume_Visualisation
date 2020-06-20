#pragma once // (c) 2020 Lukas Brunner

#include "Common/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

class ComputeBuffers;
union PitCollection;
struct TextureResource;

class PropagationShader final
{
    void destroy();

public:
    PropagationShader(ID3D12Device* device);
    ~PropagationShader();

#pragma pack(push, 1)
    struct DrawData
    {
        static const unsigned int bufferWidth = 10; // has to be the same as in InitializationShader.h and the respective .hlsl files
        static const unsigned int threadGroupSizeX = 8;
        static const unsigned int threadGroupSizeY = 8;
        Vec2 textureResolution;
        Vec3 volumeTexelDimensions;
        float voxelsPerCm;
        float remainingSlices;
        float sliceWidth;
    };
#pragma pack(pop)
    // propagates light and viewing rays through the volume and returns the amount of slices which was not yet able to process, returns also the last written 
    // color buffer via outColorBufferResource and outColorBufferDescriptorHandle
    unsigned int draw(ID3D12GraphicsCommandList* cmd, const DrawData& data, ComputeBuffers* computeBuffers, const PitCollection* pitCollection, const TextureResource* volumeTexture,
        ID3D12Resource*& outColorBufferResource, PAL::DescriptorHandle::ResidentHandles& outColorBufferDescriptorHandle);

private:
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    int slicesPerInvocation;
};

BRWL_RENDERER_NS_END