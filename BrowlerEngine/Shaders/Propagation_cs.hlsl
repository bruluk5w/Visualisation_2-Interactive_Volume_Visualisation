cbuffer constants : register(b0)
{
    float sliceWidth;
    float worldSpaceToNormalizedVolume;
};

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
    static const uint bufferWidth = 10; // in pixel
    
    const float3 read_idx = float3(DTid.xy + bufferWidth, 0);
    const uint2 write_idx = DTid.xy + bufferWidth;
    // light propagation
    const float3 lightOld = lightBufferRead.Load(read_idx).xyz;
    const float3 lightNew = lightOld;
    lightBufferWrite[write_idx].xyz = lightNew;
    
    const float3 lightDirectionOld = lightDirectionBufferRead.Load(read_idx).xyz;
    const float3 lightDirectionNew = lightDirectionOld;
    lightDirectionBufferWrite[write_idx].xyz = lightDirectionNew;
    
    
    // viewing ray propagation
    const float3 viewingRayPositionOld = viewingRayPositionBufferRead.Load(read_idx).xyz;
    const float3 viewingRayDirectionOld = viewingRayDirectionBufferRead.Load(read_idx).xyz;
    const float4 colorOld = colorBufferRead.Load(read_idx);
    const float4 colorNew = colorOld;
    
    const float3 mediumOld = mediumBufferRead.Load(read_idx).xyz;
    const float3 mediumNew = mediumOld;
    mediumBufferWrite[write_idx].xyz = mediumNew;
    
    //Advance viewing ray in world space
    const float3 viewingRayPositionNew = viewingRayPositionOld + viewingRayDirectionOld * sliceWidth;
    const float3 viewingRayDirectionNew = viewingRayDirectionOld; // todo
    
    viewingRayPositionBufferWrite[write_idx].xyz = viewingRayPositionNew;
    viewingRayDirectionBufferWrite[write_idx].xyz = viewingRayDirectionNew; // has to be normalized
    
    // Sampling the volume
    // We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/4 of the normalized range is utilized.
    const float rawScalarSampleOld = volumeTexture.SampleLevel(volumeSampler, (viewingRayPositionOld * worldSpaceToNormalizedVolume) + 0.5f, 0) * 4;
    const float rawScalarSampleNew = volumeTexture.SampleLevel(volumeSampler, (viewingRayPositionNew * worldSpaceToNormalizedVolume) + 0.5f, 0) * 4;
    const float refractionContrib = refractionIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
    const float opacityContrib = opacityIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
    // todo: change to rgb:
    const float particleColorContrib = particleColIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
    const float mediumContrib = mediumIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
    
    colorBufferWrite[write_idx] = colorNew + opacityContrib *100; //* sliceWidth;
    colorBufferWrite[write_idx].w = 1;
    
}