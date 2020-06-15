cbuffer constants: register(b0)
{
    float2 textureSizeWorldSpace;
    float2 textureResolution;
    float3 horizontalPlaneDirection; // normalized
    float dummy;
    float3 verticalPlaneDirection;  // normalized
    float dummy1;
    float3 topLeft;
    float dummy2;
    float3 eye;
};

RWTexture2D<float3> viewDirectionBuffer : register(u0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    const float2 halfPixel = textureSizeWorldSpace / textureResolution * 0.5f;
    const float2 offset = halfPixel + float2(DTid.xy) * textureSizeWorldSpace.x / textureResolution;

    const float3 pixelWorldSpace = topLeft + 
        offset.x * horizontalPlaneDirection +
        offset.y * verticalPlaneDirection;
    
    viewDirectionBuffer[DTid.xy] = normalize(pixelWorldSpace - eye);
}