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
        static const unsigned int threadGroupSizeX = 16;
        static const unsigned int threadGroupSizeY = 16;
        static const unsigned int num32BitValues = 2;
        Vec2 textureResolution;
        float numSlices;
        float sliceWidth;
    };
#pragma pack(pop)

    void PropagationShader::draw(ID3D12GraphicsCommandList* cmd, const DrawData& data,
        ComputeBuffers* computeBuffers, const PitCollection* pitCollection, const TextureResource* volumeTexture);

private:
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
};

BRWL_RENDERER_NS_END