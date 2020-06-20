cbuffer constants : register(b0)
{
    float sliceWidth;
    float3 worldSpaceToNormalizedVolume;
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

static const float PI = 3.14159265f;

// inputs are assumed to be normalized
// fromIdxOfRefrac and toIdxOfRefrac assumed > 0 
float4 cookTorrance(float3 normal, float3 view, float3 light, float fromIdxOfRefrac, float toIdxOfRefrac, float roughness)
{
    float3 halfVector = normalize(light + view);
    float angleOfIncidence = dot(normal, light); // cos of the angle between normal and view
    float angleOfReflection = dot(normal, view); // cos of the angle between normal and light
    float normalDotHalf = dot(normal, halfVector);
    float viewDotHalf = dot(view, halfVector);
    // Compute the geometric term 
    float G1 = (2.0f * normalDotHalf * angleOfIncidence) / viewDotHalf;
    float G2 = (2.0f * normalDotHalf * angleOfReflection) / viewDotHalf;
    //float  geometricAttenuation = min( 1.0f, max( 0.0f, min( G1, G2 ) ) );    
    float geometricAttenuation = min(1.0f, min(G1, G2));
    // Schlick's approximation of the fresnel term
    float r0 = (fromIdxOfRefrac - toIdxOfRefrac) / (fromIdxOfRefrac + toIdxOfRefrac); ///reflection coefficient for light incoming parallel to normal
    float fresnel = r0 + (1.f - r0) * pow(1.f - angleOfIncidence, 5.0f); // todo: mybe faster with 5 multiplications
    // Roughness calculated with Beckmann distribution
    float roughnessSq = roughness * roughness;
    float normalDotHalfSq = normalDotHalf * normalDotHalf;
    float distributionFactor = exp(-(1.f - normalDotHalfSq) / (roughnessSq * normalDotHalfSq)) / (PI * roughnessSq * normalDotHalfSq * normalDotHalfSq);
    // final cook torrance spceular term 
    return (distributionFactor * fresnel * geometricAttenuation) / (PI * angleOfReflection * angleOfIncidence);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    static const uint bufferWidth = 10; // in pixel
    
    const int3 read_idx = int3(DTid.xy + bufferWidth, 0);
    const uint2 write_idx = DTid.xy + bufferWidth;
    // light propagation
    const float3 lightOld = lightBufferRead.Load(read_idx).xyz;
    const float3 lightDirectionOld = lightDirectionBufferRead.Load(read_idx).xyz;
    // intersect with previous light plane
    const float3 lightNew = lightOld;
    lightBufferWrite[write_idx].xyz = lightNew;
    
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
    
    colorBufferWrite[write_idx] = colorNew + rawScalarSampleNew;
    colorBufferWrite[write_idx].w = 1;
    
}