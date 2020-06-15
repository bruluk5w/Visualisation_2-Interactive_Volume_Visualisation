#pragma once // (c) 2020 Lukas Brunner

BRWL_NS
struct BBox;
struct VertexData;
BRWL_NS_END

BRWL_RENDERER_NS


struct TextureResource;
union PitCollection;
class InitializationShader;

namespace PAL
{
    extern DXGI_FORMAT g_RenderTargetFormat;
}

class MainShader final
{
    struct TriangleList
    {
        TriangleList();
        bool load(ID3D12Device* device, const VertexData* vertices, unsigned int numVertices);
        void destroy();

        ComPtr<ID3D12Resource> vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        unsigned int vertexBufferLength;
        bool loaded;
    };

public:
    MainShader();
    ~MainShader();

    bool create(ID3D12Device* device);
    void render();
    void destroy();

    struct DrawData {
        const BBox* volumeDimensions;
        const TextureResource* volumeTexture;
        const PitCollection* pitCollection;
        const float voxelsPerCm;
        bool drawAssetBounds;
        bool drawViewingVolume;
    };

    void draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmd, const DrawData& data);

private:
    void bindVertexBuffer(ID3D12GraphicsCommandList* cmd, const TriangleList& list);

    ComPtr<ID3D12RootSignature>  mainRootSignature;
    ComPtr<ID3D12PipelineState>  mainPipelineState;
    ComPtr<ID3D12RootSignature>  guidesRootSignature;
    ComPtr<ID3D12PipelineState>  guidesPipelineState;

    TriangleList viewingPlane;
    TriangleList assetBounds;

    std::unique_ptr<InitializationShader> initializationShader;

    bool initialized;
};

BRWL_RENDERER_NS_END