#pragma once

#include "Renderer/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

struct TextureResource;

class InitializationShader final
{
    void destroy();

public:
    InitializationShader(ID3D12Device* device);
    ~InitializationShader();

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
    };

    void draw(ID3D12GraphicsCommandList* cmd, const ShaderConstants& constants, TextureResource* texToinitialize);

    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    PAL::DescriptorHeap::Handle* uavHandle;
};

BRWL_RENDERER_NS_END