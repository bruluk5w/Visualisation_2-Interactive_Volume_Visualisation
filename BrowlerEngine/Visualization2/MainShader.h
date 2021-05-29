#pragma once // (c) 2020 Lukas Brunner

#include "Renderer/RendererFwd.h"
#include "Renderer/TextureHandleFwd.h"

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
    };

    struct PsConstants
    {
        float numSlices;
        Vec3 deltaSlice;
    };
#pragma pack(pop)

public:
    MainShader();
    ~MainShader();

    bool create(Renderer* renderer);
    void render(ID3D12GraphicsCommandList* cmd);
    void destroy(Renderer* renderer);

    struct DrawData {
        static const unsigned int gatherTextureSize = 1024;
        TextureHandle& volumeTexturehandle;
        PitCollection& pitCollection;
        const float numSlicesPerVoxel;
        bool drawAssetBounds;
        bool drawViewingVolume;
        bool drawOrthographicXRay;
        bool hasViewChanged;
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

    void draw(ID3D12Device* device, ID3D12GraphicsCommandList* cmd, DrawData& data);
    unsigned int getNumRemainingSlices() const { return remainingSlices; }
private:
    void bindVertexBuffer(ID3D12GraphicsCommandList* cmd, const TriangleList& list);
    void switchToCompute(ID3D12GraphicsCommandList* cmd, MainShader::DrawData& data);
    void switchToPixelShader(ID3D12GraphicsCommandList* cmd, MainShader::DrawData& data);

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

    unsigned int remainingSlices;

    bool initialized;
    bool areResourcesUsedByPixelShader;
};

BRWL_RENDERER_NS_END