#pragma once // (c) 2020 Lukas Brunner


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
    void draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmd);

    void destroy();

private:
    void setupRenderState(ID3D12GraphicsCommandList* cmd);

    ComPtr<ID3D12RootSignature>  rootSignature;
    ComPtr<ID3D12PipelineState>  pipelineState;

    struct TriangleList
    {
        TriangleList() : vertexBuffer(nullptr), vertexBufferLength(0), vertexBufferView{}
        { }

        void destroy() {
            vertexBuffer = nullptr;
            vertexBufferLength = 0;
            memset(&vertexBufferView, 0, sizeof(vertexBufferView));
        }

        ComPtr<ID3D12Resource> vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        size_t vertexBufferLength;
    };

    TriangleList viewingPlane;

    bool initialized;
};

BRWL_RENDERER_NS_END