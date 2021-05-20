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
};

RWTexture2D<float4> lightBuffer : register(u0);
RWTexture2D<float4> lightDirectionBuffer : register(u1);
RWTexture2D<float4> colorBuffer : register(u2);
RWTexture2D<float4> mediumBuffer : register(u3);
RWTexture2D<float4> viewingRayPositionBuffer : register(u4);
RWTexture2D<float4> viewingRayDirectionBuffer : register(u5);


[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint2 texDim;
    lightBuffer.GetDimensions(texDim.x, texDim.y); // we assume all textures to be of the same size

    lightBuffer[DTid.xy] = float4(lightColor.xyz, 1);
    lightDirectionBuffer[DTid.xy].xyz = lightDirection; // directional light only for now
    colorBuffer[DTid.xy].xyzw = float4(0.f, 0.f, 0.f, 0.f); // starting with no color and fully transparent
    mediumBuffer[DTid.xy].xyzw = float4(1.f, 1.f, 1.f, 1.f); // starting with full color because subtractive color mixing
    const float2 pixelSize = textureSizeWorldSpace / (float2)texDim;
    const float2 offset = (float2(DTid.xy)) * pixelSize;

    const float3 pixelWorldSpace = topLeft + offset.x * horizontalPlaneDirection + offset.y * verticalPlaneDirection;
    viewingRayPositionBuffer[DTid.xy].xyz = pixelWorldSpace;
    viewingRayDirectionBuffer[DTid.xy].xyz = normalize(pixelWorldSpace - eye); // in world space
}