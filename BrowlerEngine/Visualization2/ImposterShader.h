#pragma once // (c) 2020 Lukas Brunner

#include "Common/PAL/DescriptorHeap.h"

BRWL_RENDERER_NS

struct TextureResource;

class ImposterShader final
{
    void destroy();

public:
    ImposterShader(ID3D12Device* device, const D3D12_INPUT_LAYOUT_DESC& inputLayout);
    ~ImposterShader();

#pragma pack(push, 1)
    struct VsConstants
    {
        static const unsigned int num32BitValues = 20;
        Mat4 modelviewProjection;
        Vec2 uvOffset;
        Vec2 uvRangeScale;
    };
#pragma pack(pop)

    // Sets the state for drawing the plane with the result texture.
    // We only set states, drawing is handled by MainShader since it holds the vertex buffer. 
    void setupDraw(ID3D12GraphicsCommandList* cmd, const VsConstants& vsConstants, const PAL::DescriptorHandle::NativeHandles& outColorBufferDescriptorHandle);

private:
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
};

BRWL_RENDERER_NS_END