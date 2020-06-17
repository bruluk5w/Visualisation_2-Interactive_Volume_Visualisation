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

RWTexture2D<float3> lightBuffer : register(u0);
RWTexture2D<float3> lightDirectionBuffer : register(u1);
RWTexture2D<float3> colorBuffer : register(u2);
RWTexture2D<float3> mediumBuffer : register(u3);
RWTexture2D<float3> viewingRayPositionBuffer : register(u4);
RWTexture2D<float3> viewingRayDirectionBuffer : register(u5);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    const float2 halfPixel = textureSizeWorldSpace / textureResolution * 0.5f;
    const float2 offset = halfPixel + float2(DTid.xy) * textureSizeWorldSpace.x / textureResolution;

    const float3 pixelWorldSpace = topLeft + 
        offset.x * horizontalPlaneDirection +
        offset.y * verticalPlaneDirection;
    
    viewingRayDirectionBuffer[DTid.xy] = normalize(pixelWorldSpace - eye);
}