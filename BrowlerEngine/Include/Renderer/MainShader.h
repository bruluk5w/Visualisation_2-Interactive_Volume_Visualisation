#pragma once // (c) 2020 Lukas Brunner

#include "VertexShader_vs.h"
#include "PixelShader_ps.h"

#include "Renderer/PAL/d3dx12.h"

BRWL_RENDERER_NS

namespace PAL
{
    extern DXGI_FORMAT g_RenderTargetFormat;
}

class MainShader final
{
public:
    MainShader();
    bool create(ID3D12Device* device);
	

    void render();

    void destroy();

private:
    ComPtr<ID3D12RootSignature>  rootSignature;
    ComPtr<ID3D12PipelineState>  pipelineState;

    bool initialized;
};

BRWL_RENDERER_NS_END