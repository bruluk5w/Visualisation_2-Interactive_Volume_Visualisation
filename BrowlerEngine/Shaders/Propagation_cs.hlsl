cbuffer constants : register(b0)
{
    float3 bboxmin;
    float3 bboxmax;
    float3 deltaSlice;
};

SamplerState preintegrationSampler : register(s0);
SamplerState volumeSampler : register(s1);

RWTexture2D<float4> lightBufferWrite : register(u0);
RWTexture2D<float4> lightDirectionBufferWrite : register(u1);
RWTexture2D<float4> colorBufferWrite : register(u2);
RWTexture2D<float4> mediumBufferWrite : register(u3);
RWTexture2D<float4> viewingRayPositionBufferWrite : register(u4);
RWTexture2D<float4> viewingRayDirectionBufferWrite : register(u5);


RWTexture2D<float4> lightBufferRead : register(u6);
RWTexture2D<float4> lightDirectionBufferRead : register(u7);
RWTexture2D<float4> colorBufferRead : register(u8);
RWTexture2D<float4> mediumBufferRead : register(u9);
RWTexture2D<float4> viewingRayPositionBufferRead : register(u10);
RWTexture2D<float4> viewingRayDirectionBufferRead : register(u11);

Texture2D<float> refractionIntegTex : register(t0);
Texture2D<float> particleColIntegTex : register(t1);
Texture2D<float> opacityIntegTex : register(t2);
Texture2D<float> mediumIntegTex : register(t3);
Texture3D<float> volumeTexture : register(t4);

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



float3 getUVCoordinates(float3 coordinate)
{
	return (coordinate - bboxmin) / (bboxmax - bboxmin);
}


float3 integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).r;
}




[numthreads(16, 16, 1)]

void main ( uint3 DTid : SV_DispatchThreadID )

{
	const uint3 read_idx = int3(DTid.xy, 0);
	const uint2 write_idx = DTid.xy;
    const float sliceDepth = length(deltaSlice);
    
    /*
        Light Propagation
    */
    
    // TODO: Add refraction here
    const float3 currentLightDirection = lightDirectionBufferRead.Load(read_idx).xyz;
    const float3 currentLightIntensity = lightBufferRead.Load(read_idx).xyz;
    
    lightDirectionBufferWrite[write_idx].xyz = currentLightDirection;
    lightBufferWrite[write_idx].xyz = currentLightIntensity;


    /*
        Viewing Ray Propagation
    */
    const float3 currentViewingRayPosition = viewingRayPositionBufferRead.Load(read_idx).xyz;
    const float3 currentViewingRayDirection = viewingRayDirectionBufferRead.Load(read_idx).xyz;
    
    // We know that the write buffer holds the previous pixel's viewing ray position
    const float3 previousViewingRayPosition = viewingRayPositionBufferWrite.Load(read_idx).xyz;
    
    float4 currentColor = colorBufferRead.Load(read_idx);
    float3 currentMediumColor = mediumBufferRead.Load(read_idx).xyz;
    
    float3 diffuseLight = lightBufferRead.Load(read_idx).xyz; // Need to calculate values first
    float3 specularLight = float3(0, 0, 0); //TODO: specularBDRF(currentLightDirection, currentViewingRayDirection, delta f)
    
    
    // Sampling the volume
    // We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/8 of the normalized range is utilized.
    const float previousScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinates(previousViewingRayPosition), 0) * 8;
    const float currentScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinates(currentViewingRayPosition), 0) * 8;
    
    float3 color = integrationTable(previousScalarSample, currentScalarSample, particleColIntegTex);
    float3 alpha = integrationTable(previousScalarSample, currentScalarSample, opacityIntegTex);
    float3 medium = integrationTable(previousScalarSample, currentScalarSample, mediumIntegTex);
    float3 deleteme = integrationTable(previousScalarSample, currentScalarSample, refractionIntegTex);
    
    float3 nextColor = currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (alpha * color * diffuseLight + specularLight);
    float nextAlpha = currentColor.w + (1 - currentColor.w) * alpha.x;
    float3 nextMediumColor = currentMediumColor * medium;
    
    // Advance viewing ray in world space
    const float3 nextViewingRayPosition = currentViewingRayPosition + currentViewingRayDirection * dot(currentViewingRayDirection, deltaSlice);
    const float3 nextViewingRayDirection = currentViewingRayDirection; // todo
    
   
    
    viewingRayPositionBufferWrite[write_idx].xyz = nextViewingRayPosition;
    viewingRayDirectionBufferWrite[write_idx].xyz = nextViewingRayDirection; // has to be normalized
    colorBufferWrite[write_idx].xyz = nextColor;
    colorBufferWrite[write_idx].w = nextAlpha;
    mediumBufferWrite[write_idx].xyz = nextMediumColor;
}