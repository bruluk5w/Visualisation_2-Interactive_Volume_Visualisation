cbuffer constants: register(b0)
{
    float2 textureSizeWorldSpace;
    float2 textureResolution;
    float3 horizontalPlaneDirection; // normalized
    float pad0;
    float3 verticalPlaneDirection;  // normalized
    float pad1;
    float3 topLeft;
    float pad2;
    float3 eye;
    float pad3;
    float3 lightDirection;
    float pad4;
    float4 lightColor;
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
    lightBuffer[DTid.xy].xyzw = lightColor;
    lightDirectionBuffer[DTid.xy].rgb = lightDirection; // directional light only for now
    colorBuffer[DTid.xy].xyzw = float4(0.f, 0.f, 0.f, 0.f); // starting with no color and fully transparent
    // ?? is this correct?
    mediumBuffer[DTid.xy].xyzw = float4(0.f, 0.f, 0.f, 0.f); // starting with no color and fully transparent
    
    const float2 halfPixel = textureSizeWorldSpace / textureResolution * 0.5f;
    const float2 offset = halfPixel + float2(DTid.xy) * textureSizeWorldSpace.x / textureResolution;

    const float3 pixelWorldSpace = topLeft + 
        offset.x * horizontalPlaneDirection +
        offset.y * verticalPlaneDirection;
    viewingRayPositionBuffer[DTid.xy].xyz = pixelWorldSpace;
    viewingRayDirectionBuffer[DTid.xy].xyz = normalize(pixelWorldSpace - eye); // in world space
}