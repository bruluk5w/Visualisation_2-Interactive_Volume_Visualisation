#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/RendererFwd.h"

BRWL_NS
struct BBox;
struct VertexData;
BRWL_NS_END

BRWL_RENDERER_NS


struct TextureResource;
union PitCollection;
class InitializationShader;
class PropagationShader;
class ComputeBuffers;
class ImposterShader;

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
#pragma pack(push, 1)
     struct VsConstants
    {
        Mat4 modelMatrix;
        Mat4 viewProjection;
        float voxelsPerCm;
    };

    struct PsConstants
    {
        float voxelsPerCm;
        float numSlices;
        float pad0;
        float pad1;
        Vec3 deltaSlice;
    };
#pragma pack(pop)

public:
    MainShader();
    ~MainShader();

    bool create(Renderer* renderer);
    void render();
    void destroy();

    struct DrawData {
        static const unsigned int gatherTextureSize = 1024;
        const BBox* volumeDimensions;
        const TextureResource* volumeTexture;
        const PitCollection* pitCollection;
        const float voxelsPerCm;
        bool drawAssetBounds;
        bool drawViewingVolume;
        bool drawOrthographicXRay;
        struct Light
        {
            enum class Type : uint8_t
            {
                DIRECTIONAL
            } type;
            Vec3 coords;
            Vec4 color;
        } light;
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
    std::unique_ptr<PropagationShader> propagationShader;
    std::unique_ptr<ComputeBuffers> computeBuffers;
    std::unique_ptr<ImposterShader> imposterShader;

    bool initialized;
};

BRWL_RENDERER_NS_END