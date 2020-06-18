#pragma once // (c) 2020 Lukas Brunner

#include "Common/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

class ComputeBuffers;

class InitializationShader final
{
    void destroy();

public:
    InitializationShader(ID3D12Device* device);
    ~InitializationShader();

#pragma pack(push, 1)
    struct ShaderConstants
    {
        static const unsigned int threadGroupSizeX = 8;
        static const unsigned int threadGroupSizeY = 8;
        static const unsigned int num32BitValues = 19;

        Vec2 textureSizeWorldSpace;
        Vec2 textureResolution;
        Vec3 horizontalPlaneDirection; // normalized
        float padding0;
        Vec3 verticalPlaneDirection;  // normalized
        float padding1;
        Vec3 topLeft;
        float padding2;
        Vec3 eye;
        float padding3;
        Vec3 lightDirection;
        float padding4;
        Vec4 lightColor;
    };
#pragma pack(pop)

    void draw(ID3D12GraphicsCommandList* cmd, const ShaderConstants& constants, ComputeBuffers* computeBuffers);
private:

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
};

BRWL_RENDERER_NS_END