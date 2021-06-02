#include "Common.hlsli"

cbuffer constants : register(b0)
{
    float3 bboxmin;
    float texDimToUV; // 1 / number of pixel per side of slice plane
    float3 bboxmax;
    float worldDimToUV; // 1 / world space width of slice plane
    float3 deltaSlice;
    float texelDim; // size of a texel on the slice plane
    float3 planeRight; // normalized
    float backgroundScale; // scale of the background checker board
    float3 planeDown; // normalized
    float3 topLeft; //topLeft of current slice
};

SamplerState preintegrationSampler : register(s0);
SamplerState volumeSampler : register(s1);
SamplerState lightSampler : register(s2);

RWTexture2D<float4> lightBufferWrite : register(u0);
RWTexture2D<float4> lightDirectionBufferWrite : register(u1);
RWTexture2D<float4> colorBufferWrite : register(u2);
RWTexture2D<float4> mediumBufferWrite : register(u3);
RWTexture2D<float4> viewingRayPositionBufferWrite : register(u4);
RWTexture2D<float4> viewingRayDirectionBufferWrite : register(u5);


Texture2D<float4> lightBufferRead : register(t0);           // have to be SRVs to support light sampling during shading along viewing ray
Texture2D<float4> lightDirectionBufferRead : register(t1);  //
RWTexture2D<float4> colorBufferRead : register(u6);                 // may stay UAVs because Load and Store are probably enough for data that is full aligned with the viewing rays
RWTexture2D<float4> mediumBufferRead : register(u7);                //
RWTexture2D<float4> viewingRayPositionBufferRead : register(u8);    //
RWTexture2D<float4> viewingRayDirectionBufferRead : register(u9);   //

Texture2D<float> refractionLut : register(t2);
Texture2D<float4> particleColIntegTex : register(t3);
Texture2D<float> opacityIntegTex : register(t4);
Texture2D<float4> mediumIntegTex : register(t5);
Texture3D<float> volumeTexture : register(t6);

//static const float PI = 3.14159265f;

//// inputs are assumed to be normalized
//// fromIdxOfRefrac and toIdxOfRefrac assumed > 0 
//float4 cookTorrance(float3 normal, float3 view, float3 light, float fromIdxOfRefrac, float toIdxOfRefrac, float roughness)
//{
//    float3 halfVector = normalize(light + view);
//    float angleOfIncidence = dot(normal, light); // cos of the angle between normal and view
//    float angleOfReflection = dot(normal, view); // cos of the angle between normal and light
//    float normalDotHalf = dot(normal, halfVector);
//    float viewDotHalf = dot(view, halfVector);
//    // Compute the geometric term 
//    float G1 = (2.0f * normalDotHalf * angleOfIncidence) / viewDotHalf;
//    float G2 = (2.0f * normalDotHalf * angleOfReflection) / viewDotHalf;
//    //float  geometricAttenuation = min( 1.0f, max( 0.0f, min( G1, G2 ) ) );    
//    float geometricAttenuation = min(1.0f, min(G1, G2));
//    // Schlick's approximation of the fresnel term
//    float r0 = (fromIdxOfRefrac - toIdxOfRefrac) / (fromIdxOfRefrac + toIdxOfRefrac); ///reflection coefficient for light incoming parallel to normal
//    float fresnel = r0 + (1.f - r0) * pow(1.f - angleOfIncidence, 5.0f); // todo: mybe faster with 5 multiplications
//    // Roughness calculated with Beckmann distribution
//    float roughnessSq = roughness * roughness;
//    float normalDotHalfSq = normalDotHalf * normalDotHalf;
//    float distributionFactor = exp(-(1.f - normalDotHalfSq) / (roughnessSq * normalDotHalfSq)) / (PI * roughnessSq * normalDotHalfSq * normalDotHalfSq);
//    // final cook torrance spceular term 
//    return (distributionFactor * fresnel * geometricAttenuation) / (PI * angleOfReflection * angleOfIncidence);
//}


float3 getUVCoordinatesVolume(float3 worldCoord)
{
	return (worldCoord - bboxmin) / (bboxmax - bboxmin);
}

void sampleLightTextures(float3 worldCoord, out float3 lightDirection, out float3 lightIntensity)
{
    const float3 offset = (worldCoord - topLeft) * worldDimToUV;
    const float2 uv = float2(dot(offset, planeRight), dot(offset, planeDown)) + 0.5f * texDimToUV; // + texDimToUV to sample in the center of the texel
    lightDirection = lightDirectionBufferRead.SampleLevel(lightSampler, uv, 0).xyz;
    lightIntensity = lightBufferRead.SampleLevel(lightSampler, uv, 0).xyz;
}

float lut(float scalarSample, Texture2D<float> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(scalarSample, 0.5f), 0).r * 0.01; // *0.05 because else refraction is too powerful
}

float integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).r * length(deltaSlice);
}

float3 integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float4> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).rgb * length(deltaSlice);
}


void reconstructVolumeGradient(float3 worldPos, out float3 gradient, out float3 refractionGradient)
{
    const float stepLength = 1.0f;
    const float h = 1.0f / (2.f * stepLength);
    const float left =    volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(stepLength, 0, 0)), 0) * 8;
    const float right =   volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(stepLength, 0, 0)), 0) * 8;
    const float top =     volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(0, stepLength, 0)), 0) * 8;
    const float bottom =  volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(0, stepLength, 0)), 0) * 8;
    const float front =   volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(0, 0, stepLength)), 0) * 8;
    const float back =    volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(0, 0, stepLength)), 0) * 8;
            
    gradient = float3(right - left, bottom - top, back - front);
    if (any(gradient))
        gradient = normalize(gradient);
    
    const float leftIOR = lut(left, refractionLut);
    const float rightIOR = lut(right, refractionLut);
    const float topIOR = lut(top, refractionLut);
    const float bottomIOR = lut(bottom, refractionLut);
    const float frontIOR = lut(front, refractionLut);
    const float backIOR = lut(back, refractionLut);
    
    refractionGradient = float3(rightIOR - leftIOR, bottomIOR - topIOR, backIOR - frontIOR) * h;
}

float2 projectOnLightPlane(float3 worldVec)
{
    //const float3 normalizedDeltaSlice = normalize(deltaSlice);
    //return (worldVec - dot(worldVec, normalizedDeltaSlice) * normalizedDeltaSlice);
    return float2(dot(worldVec, planeRight), dot(worldVec, planeDown));

}

float calculateLightDirectionDerivative(float2 lightUV, Texture2D<float4> tex)
{
    const float stepLength = texDimToUV;
    const float h = 1.0f / (2.f * stepLength);
    const float2 left   = projectOnLightPlane(lightDirectionBufferRead.SampleLevel(lightSampler, lightUV - float2(stepLength, 0), 0).xyz);
    const float2 right  = projectOnLightPlane(lightDirectionBufferRead.SampleLevel(lightSampler, lightUV + float2(stepLength, 0), 0).xyz);
    const float2 top    = projectOnLightPlane(lightDirectionBufferRead.SampleLevel(lightSampler, lightUV - float2(0, stepLength), 0).xyz);
    const float2 bottom = projectOnLightPlane(lightDirectionBufferRead.SampleLevel(lightSampler, lightUV + float2(0, stepLength), 0).xyz);
    
    return abs(determinant(float2x2((right - left) * h, (bottom - top) * h))); // absolute value of jacobian
}

float3 blinnPhongSpecular(float3 lightDir, float3 viewDir, float3 normal, float3 lightIntensity, float reflectivity)
{
    const float3 halfVector = normalize(lightDir + viewDir);
    return pow(max(dot(normal, halfVector), 0.00001), reflectivity) * lightIntensity;
}


[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void main ( uint3 DTid : SV_DispatchThreadID )

{
	const int3 read_idx = int3(DTid.xy, 0);
	const uint2 write_idx = DTid.xy;
    const float sliceDepth = length(deltaSlice);
    
    /*
     *    Light Propagation
     */
    
    {
    
    
        const float3 currentLightDirection = lightDirectionBufferRead.Load(read_idx).xyz;
        const float3 currentLightIntensity = lightBufferRead.Load(read_idx).xyz;
        
        float len = dot(currentLightDirection, normalize(deltaSlice));
        const float3 currentLightWorldPos = topLeft + (((float) DTid.x) * planeRight + ((float) DTid.y) * planeDown) * texelDim;
        const float3 previousLightWorldPos = currentLightWorldPos - currentLightDirection * length(deltaSlice) / len;
    

        const float3 currentOffset = (currentLightWorldPos - topLeft) * worldDimToUV;
        const float2 currentLightUV = float2(dot(currentOffset, planeRight), dot(currentOffset, planeDown)) + 0.5f * texDimToUV; // todo: is this correct, shifts somehow the light texture to the bottom right
        const float3 previousOffset = (previousLightWorldPos - topLeft) * worldDimToUV;
        const float2 previousLightUV = float2(dot(previousOffset, planeRight), dot(previousOffset, planeDown)) + 0.5f * texDimToUV;
        
        // intensity correction
        
        float previousS = 1; // calculateLightDirectionDerivative(previousLightUV, lightBufferRead); // jacobian of previous light texture
        
        // from sampleLightTextures(previousLightWorldPos, previousLightDirection, previousLightIntensity);
        float3 previousLightDirection = lightDirectionBufferRead.SampleLevel(lightSampler, previousLightUV, 0).xyz;
        float3 previousLightIntensity = lightBufferRead.SampleLevel(lightSampler, previousLightUV, 0).xyz;
        
        // derivative of current light texture
        // todo
        float currentS = previousS;//calculateLightDirectionDerivative(previousLightUV, lightDirectionBufferRead);
        float intensityCorrection = abs(currentS) < 0.000001 ? 1 : previousS / currentS;
        
        
        // calculate new light intensity by attenuating and correcting with intensityCorrection
        const float previousScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(previousLightWorldPos), 0) * 8;
        const float currentScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(currentLightWorldPos), 0) * 8;
        float alpha = saturate(integrationTable(previousScalarSample, currentScalarSample, opacityIntegTex));
        float3 medium = saturate(integrationTable(previousScalarSample, currentScalarSample, mediumIntegTex));
        const float3 nextLightIntensity = previousLightIntensity * intensityCorrection * (1 - alpha) * (1 - medium);

        // refract light ray
        float3 dummy;
        float3 refractionGradient;
        reconstructVolumeGradient(currentLightWorldPos, dummy, refractionGradient);
        const float3 nextLightDirection = normalize(previousLightDirection + sliceDepth * refractionGradient);
        
        lightDirectionBufferWrite[write_idx].xyz = nextLightDirection;
        lightBufferWrite[write_idx].xyz = nextLightIntensity;
    }


    /*
     *   Viewing Ray Propagation
     */
    const float4 currentViewingRayPosition = viewingRayPositionBufferRead.Load(read_idx); // w component holds previous scalar sample
    const float3 currentViewingRayDirection = viewingRayDirectionBufferRead.Load(read_idx).xyz;
    
    float4 currentColor = colorBufferRead.Load(read_idx);
    
    if (currentColor.w >= 1)
    { // Would be sufficient to do it only once and write the second plane/layer into the next Buffer
        colorBufferWrite[write_idx] = currentColor;
        return;
    }
    
    // Sampling light information
    float3 lightDirection;
    float3 lightIntensity;
    sampleLightTextures(currentViewingRayPosition.xyz, lightDirection, lightIntensity);
    
    float3 currentMediumColor = mediumBufferRead.Load(read_idx).xyz;
    
    if (!rayIntersectsVolume(currentViewingRayPosition.xyz, currentViewingRayDirection, bboxmax, bboxmin))
    {
        //float3 nextColor                = currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (alpha * color * lightIntensity * lambertianTerm + specularLight);
        float3 color = checkerBoard(currentViewingRayPosition.xyz, backgroundScale);
        float lambertianTerm = max(dot(float3(0, 0, 1), lightDirection), 0);
        colorBufferWrite[write_idx] = float4(currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (color * lightIntensity * lambertianTerm), 1);
               return;
    }
    
    // reconstruct gradient of scalar field
    float3 gradient;
    float3 refractionGradient;
    reconstructVolumeGradient(currentViewingRayPosition.xyz, gradient, refractionGradient);
   
    // Sampling the volume
    // We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/8 of the normalized range is utilized.
    const float currentScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(currentViewingRayPosition.xyz), 0) * 8;
    // simulate repeat border behaviour if first value is not available
    const float previousScalarSample = currentViewingRayPosition.w < 0 ? currentScalarSample : currentViewingRayPosition.w;
    
   
    float currentIndexOfRefraction = lut(currentScalarSample, refractionLut);

    
    // Advance viewing ray in world space
    float len = dot(currentViewingRayDirection, normalize(deltaSlice));
    const float3 nextViewingRayPosition = currentViewingRayPosition.xyz + currentViewingRayDirection * length(deltaSlice) / len;
    const float3 nextViewingRayDirection = currentViewingRayDirection + sliceDepth * refractionGradient;
    
    const float nextScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(nextViewingRayPosition), 0) * 8;
    
    float3 color = saturate(integrationTable(previousScalarSample, currentScalarSample, particleColIntegTex));
    float alpha = saturate(integrationTable(previousScalarSample, currentScalarSample, opacityIntegTex));
    float3 medium = saturate(integrationTable(previousScalarSample, currentScalarSample, mediumIntegTex));
    
    float nextIndexOfRefraction = lut(nextScalarSample, refractionLut);
    
    // the lower reflectivity the more it reflects, therefore the higher the difference, the more is subtracted
    float lambertianTerm = max(dot(gradient, lightDirection), 0);
    float3 specularLight = float3(0, 0, 0);
    if (lambertianTerm > 0)
    {
        //float reflectivity = 20 - 50 * abs(nextIndexOfRefraction - currentIndexOfRefraction);
        specularLight = blinnPhongSpecular(lightDirection, currentViewingRayDirection, gradient, 1, 20) * 100 * saturate(nextIndexOfRefraction - currentIndexOfRefraction);
    }
    
    float3 nextColor = currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (alpha * color * lightIntensity * lambertianTerm + specularLight);
    float nextAlpha = currentColor.w + (1 - currentColor.w) * alpha;
    float3 nextMediumColor = currentMediumColor * (1 - medium);
    
    viewingRayPositionBufferWrite[write_idx] = float4(nextViewingRayPosition, currentScalarSample); // we also save the current scalar sample to save looking it up again in the next frame
    viewingRayDirectionBufferWrite[write_idx].xyz = nextViewingRayDirection; // has to be normalized
    colorBufferWrite[write_idx].xyz = nextColor;
    colorBufferWrite[write_idx].w = nextAlpha;
    mediumBufferWrite[write_idx].xyz = nextMediumColor;
}