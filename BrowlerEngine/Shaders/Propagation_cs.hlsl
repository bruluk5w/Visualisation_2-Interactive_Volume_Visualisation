SamplerState preintegrationSampler : register(s0);
SamplerState volumeSampler : register(s1);

RWTexture2D<float4> lightBufferWrite : register(u0);
RWTexture2D<float4> lightDirectionBufferWrite : register(u1);
RWTexture2D<float4> colorBufferWrite : register(u2);
RWTexture2D<float4> mediumBufferWrite : register(u3);
RWTexture2D<float4> viewingRayPositionBufferWrite : register(u4);
RWTexture2D<float4> viewingRayDirectionBufferWrite : register(u5);


Texture2D<float4> lightBufferRead : register(t0);
Texture2D<float4> lightDirectionBufferRead : register(t1);
Texture2D<float4> colorBufferRead : register(t2);
Texture2D<float4> mediumBufferRead : register(t3);
Texture2D<float4> viewingRayPositionBufferRead : register(t4);
Texture2D<float4> viewingRayDirectionBufferRead : register(t5);

Texture2D<float> refractionIntegTex : register(t6);
Texture2D<float> particleColIntegTex : register(t7);
Texture2D<float> opacityIntegTex : register(t8);
Texture2D<float> mediumIntegTex : register(t9);
Texture3D<float> volumeTexture : register(t10);


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
   
}