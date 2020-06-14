#pragma once // (c) 2020 Lukas Brunner

BRWL_NS
struct BBox;
BRWL_NS_END

BRWL_RENDERER_NS


struct TextureResource;
union PitCollection;

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

    struct DrawData {
        const BBox* volumeDimensions;
        const TextureResource* volumeTexture;
        const PitCollection* pitCollection;
        const float voxelsPerCm;
    };

    void draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmd, const DrawData& data);

private:
    void setup(ID3D12GraphicsCommandList* cmd);

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
        unsigned int vertexBufferLength;
    };

    TriangleList viewingPlane;

    bool initialized;
};

BRWL_RENDERER_NS_END