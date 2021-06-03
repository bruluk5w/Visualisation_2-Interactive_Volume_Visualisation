#include "Common.hlsli"

cbuffer constants: register(b0)
{
    float3 horizontalPlaneDirection; // normalized
    float pad0;
    float3 verticalPlaneDirection;  // normalized
    float pad1;
    float3 topLeft; // World space position of top left pixel of the viewing plane
    float pad2;
    float3 eye;     // Position of camera
    float pad3;
    float3 lightDirection; // Light direction relative to world space
    float pad4;
    float4 lightColor;     // Light color
    float2 textureSizeWorldSpace;
    float3 bboxmin;
    float3 bboxmax;
};

RWTexture2D<float4> lightBuffer : register(u0);
RWTexture2D<float4> lightDirectionBuffer : register(u1);
RWTexture2D<float4> colorBuffer : register(u2);
RWTexture2D<float4> mediumBuffer : register(u3);
RWTexture2D<float4> viewingRayPositionBuffer : register(u4);
RWTexture2D<float4> viewingRayDirectionBuffer : register(u5);


[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 texDim;
    lightBuffer.GetDimensions(texDim.x, texDim.y); // we assume all textures to be of the same size
    
    lightBuffer[DTid.xy] = float4(lightColor.xyz, 1);
    lightDirectionBuffer[DTid.xy].xyz = normalize(lightDirection); // directional light
    
    mediumBuffer[DTid.xy] = float4(1.f, 1.f, 1.f, 1.f); // starting with full color because subtractive color mixing
    const float2 pixelSize = textureSizeWorldSpace / (float2) texDim;
    const float2 offset = (float2(DTid.xy)) * pixelSize;
    const float3 pixelWorldSpace = topLeft + offset.x * horizontalPlaneDirection + offset.y * verticalPlaneDirection;
    viewingRayPositionBuffer[DTid.xy].xyz = pixelWorldSpace;
    viewingRayPositionBuffer[DTid.xy].w = -1;
    
    const float3 viewingRayDirection = normalize(pixelWorldSpace - eye);
    viewingRayDirectionBuffer[DTid.xy].xyz = viewingRayDirection; // in world space
    
    if (rayIntersectsVolume(pixelWorldSpace, viewingRayDirection, bboxmax, bboxmin))
    {
        colorBuffer[DTid.xy] = float4(0.f, 0.f, 0.f, 0.f); // starting with no color and fully transparent
    }
    else
    {
        colorBuffer[DTid.xy] = float4(0.5f, 0.5f, 0.5f, 1.f); // starting with solid color
    }
    

}