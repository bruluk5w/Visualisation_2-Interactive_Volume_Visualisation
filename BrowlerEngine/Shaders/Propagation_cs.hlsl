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
    return tex.SampleLevel(preintegrationSampler, float2(scalarSample, 0.5f), 0).r * 0.1; // * 0.1 because else refraction is too powerful
}

float integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).r * length(deltaSlice);
}

float3 integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float4> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).rgb * length(deltaSlice);
}

void reconstructVolumeGradientBase(float3 worldPos, out float3 leftTopFront, out float3 rightBottomBack)
{
    const float stepLength = 1.0f;
    const float h = 1.0f / (2.f * stepLength);
    leftTopFront = h * float3(
        volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(stepLength, 0, 0)), 0) * 8,
        volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(0, stepLength, 0)), 0) * 8,
        volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(0, 0, stepLength)), 0) * 8);
    
    rightBottomBack = h * float3(
        volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(stepLength, 0, 0)), 0) * 8,
        volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(0, stepLength, 0)), 0) * 8,
        volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(0, 0, stepLength)), 0) * 8);
}

//float3 reconstructVolumeGradient(float3 worldPos)
//{
//    float3 leftTopFront;
//    float3 rightBottomBack;
//    reconstructVolumeGradientBase(worldPos, leftTopFront, rightBottomBack);

//    float3 gradient = rightBottomBack - leftTopFront;
//    if (any(gradient))
//        gradient = normalize(gradient);
    
//    return gradient;
//}

void reconstructVolumeAndRefractionGradient(float3 worldPos, out float3 gradient, out float3 refractionGradient)
{
    float3 leftTopFront;
    float3 rightBottomBack;
    reconstructVolumeGradientBase(worldPos, leftTopFront, rightBottomBack);

    gradient = rightBottomBack - leftTopFront;
    if (any(gradient))
        gradient = normalize(gradient);
        
    const float leftIOR = lut(leftTopFront.x, refractionLut);
    const float rightIOR = lut(rightBottomBack.x, refractionLut);
    const float topIOR = lut(leftTopFront.y, refractionLut);
    const float bottomIOR = lut(rightBottomBack.y, refractionLut);
    const float frontIOR = lut(leftTopFront.z, refractionLut);
    const float backIOR = lut(rightBottomBack.z, refractionLut);
    
    refractionGradient = float3(rightIOR - leftIOR, bottomIOR - topIOR, backIOR - frontIOR);
}

float3 reconstructRefractionGradient(float3 worldPos)
{
    float3 leftTopFront;
    float3 rightBottomBack;
    reconstructVolumeGradientBase(worldPos, leftTopFront, rightBottomBack);
        
    const float leftIOR = lut(leftTopFront.x, refractionLut);
    const float rightIOR = lut(rightBottomBack.x, refractionLut);
    const float topIOR = lut(leftTopFront.y, refractionLut);
    const float bottomIOR = lut(rightBottomBack.y, refractionLut);
    const float frontIOR = lut(leftTopFront.z, refractionLut);
    const float backIOR = lut(rightBottomBack.z, refractionLut);
    
    return float3(rightIOR - leftIOR, bottomIOR - topIOR, backIOR - frontIOR);
}



float2 projectOnLightPlane(float3 worldVec)
{
    return float2(dot(worldVec, planeRight), dot(worldVec, planeDown));
}


// holds fresh light directions
groupshared float3 sharedCurrentLightDirections[THREAD_GROUP_SIZE_X * THREAD_GROUP_SIZE_Y];

float calculateIntensityCorrectionFromSharedData(uint2 groupIdx, uint2 dtId)
{
    const uint2 offset = groupIdx & 1u;
    const uint2 topL = groupIdx - offset;
    const uint2 topR = topL + uint2(1, 0);
    const uint2 bottomL = topL + uint2(0, 1);
    const uint2 bottomR = topL + uint2(1, 1);
   
    const float3 texelDown = texelDim * planeDown;
    const float3 texelRight = texelDim * planeRight;
    const float2 base = dtId - offset;
    const float3 p = topLeft + (base.x * planeRight + base.y * planeDown) * texelDim; // world pos
    return texelDim * texelDim / quadrilateralArea(
        p - length(deltaSlice) * sharedCurrentLightDirections[mad(THREAD_GROUP_SIZE_X, topL.y, topL.x)],
        p - length(deltaSlice) * sharedCurrentLightDirections[mad(THREAD_GROUP_SIZE_X, bottomL.y, bottomL.x)] + texelDown,
        p - length(deltaSlice) * sharedCurrentLightDirections[mad(THREAD_GROUP_SIZE_X, bottomR.y, bottomR.x)] + texelDown + texelRight,
        p - length(deltaSlice) * sharedCurrentLightDirections[mad(THREAD_GROUP_SIZE_X, topR.y, topR.x)] + texelRight
    );
}


[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 threadID : SV_GroupThreadID)
{
	const int3 read_idx = int3(DTid.xy, 0);
	const uint2 write_idx = DTid.xy;
    const float sliceDepth = length(deltaSlice);
    
    /*
     *    Light Propagation
     */
    
    {
    
        const float3 currentLightDirection = lightDirectionBufferRead.Load(read_idx).xyz;
        //const float3 currentLightIntensity = lightBufferRead.Load(read_idx).xyz;
        
        float len = dot(currentLightDirection, normalize(deltaSlice));
        const float3 currentLightWorldPos = topLeft + (((float) DTid.x) * planeRight + ((float) DTid.y) * planeDown) * texelDim;
        const float3 previousLightWorldPos = currentLightWorldPos - currentLightDirection * length(deltaSlice) / len;
    
        const float3 previousOffset = (previousLightWorldPos - topLeft) * worldDimToUV;
        const float2 previousLightUV = float2(dot(previousOffset, planeRight), dot(previousOffset, planeDown)) + 0.5f * texDimToUV;
        const float3 previousLightDirection = lightDirectionBufferRead.SampleLevel(lightSampler, previousLightUV, 0).xyz;
        // refract light ray
        float3 refractionGradient = reconstructRefractionGradient(currentLightWorldPos);
        const float3 nextLightDirection = normalize(previousLightDirection + sliceDepth * refractionGradient);
        // write new data to shared memory
        sharedCurrentLightDirections[THREAD_GROUP_SIZE_X * threadID.y + threadID.x] = nextLightDirection;
        
        lightDirectionBufferWrite[write_idx].xyz = nextLightDirection;

        // prepare data for calculating new light intensity by attenuating and correcting with intensityCorrection
        const float previousScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(previousLightWorldPos), 0) * 8;
        const float currentScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(currentLightWorldPos), 0) * 8;
        float alpha = saturate(integrationTable(previousScalarSample, currentScalarSample, opacityIntegTex));
        float3 medium = saturate(integrationTable(previousScalarSample, currentScalarSample, mediumIntegTex));
        float3 previousLightIntensity = lightBufferRead.SampleLevel(lightSampler, previousLightUV, 0).xyz;
        
        // intensity correction
        GroupMemoryBarrier();
        const float intensityCorrection = calculateIntensityCorrectionFromSharedData(threadID.xy, DTid.xy);

        // calculate new light intensity by attenuating and correcting with intensityCorrection
        lightBufferWrite[write_idx].xyz = previousLightIntensity * intensityCorrection * (1 - alpha) * (1 - medium);;
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
    
    const float3 currentMediumColor = mediumBufferRead.Load(read_idx).xyz;
    
    if (!rayIntersectsVolume(currentViewingRayPosition.xyz, currentViewingRayDirection, bboxmax, bboxmin))
    {
        // calculate normal of the wall that we hit
        float3 invBackgroundNormal = abs(currentViewingRayPosition.xyz) - abs(bboxmax);
        if (invBackgroundNormal.x >= invBackgroundNormal.y && invBackgroundNormal.x >= invBackgroundNormal.z) invBackgroundNormal = float3(1, 0, 0);
        else if(invBackgroundNormal.y >= invBackgroundNormal.z) invBackgroundNormal = float3(0, 1, 0);
        else invBackgroundNormal = float3(0, 0, 1);
        invBackgroundNormal *= sign(currentViewingRayPosition.xyz);
        
        const float3 color = checkerBoard(currentViewingRayPosition.xyz, backgroundScale) ;
        const float lambertianTerm = max(dot(invBackgroundNormal, lightDirection), 0);
        colorBufferWrite[write_idx] = float4(currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (color * lightIntensity * lambertianTerm), 1);
        return;
    }
    
    // reconstruct gradient of scalar field
    float3 gradient;
    float3 refractionGradient;
    reconstructVolumeAndRefractionGradient(currentViewingRayPosition.xyz, gradient, refractionGradient);
   
    // Sampling the volume
    // We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/8 of the normalized range is utilized.
    const float currentScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(currentViewingRayPosition.xyz), 0) * 8;
    // simulate repeat border behaviour if first value is not available
    const float previousScalarSample = currentViewingRayPosition.w < 0 ? currentScalarSample : currentViewingRayPosition.w;
    
   
    float currentIndexOfRefraction = lut(currentScalarSample, refractionLut);

    
    // Advance viewing ray in world space
    float len = dot(currentViewingRayDirection, normalize(deltaSlice));
    const float3 nextViewingRayPosition = currentViewingRayPosition.xyz + currentViewingRayDirection * length(deltaSlice) / len;
    const float3 nextViewingRayDirection = normalize(currentViewingRayDirection + sliceDepth * refractionGradient);
    
    const float nextScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(nextViewingRayPosition), 0) * 8;
    
    float3 color  = saturate(integrationTable(previousScalarSample, currentScalarSample, particleColIntegTex));
    float  alpha  = saturate(integrationTable(previousScalarSample, currentScalarSample, opacityIntegTex));
    float3 medium = saturate(integrationTable(previousScalarSample, currentScalarSample, mediumIntegTex));
    
    float nextIndexOfRefraction = lut(nextScalarSample, refractionLut);
    
    // the lower reflectivity the more it reflects, therefore the higher the difference, the more is subtracted
    const float lambertianTerm = max(dot(gradient, lightDirection), 0);
    float3 specularLight = float3(0, 0, 0);
    if (lambertianTerm > 0)
    {
        specularLight = blinnPhongSpecular(lightDirection, currentViewingRayDirection, gradient, 1, 20) * 100 * saturate(nextIndexOfRefraction - currentIndexOfRefraction);
    }
    
    const float3 nextColor = currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (alpha * color * lightIntensity * lambertianTerm + specularLight);
    const float nextAlpha = currentColor.w + (1 - currentColor.w) * alpha;
    const float3 nextMediumColor = currentMediumColor * (1 - medium);
    
    viewingRayPositionBufferWrite[write_idx] = float4(nextViewingRayPosition, currentScalarSample); // we also save the current scalar sample to save looking it up again in the next frame
    viewingRayDirectionBufferWrite[write_idx].xyz = nextViewingRayDirection; 
    colorBufferWrite[write_idx].xyz = nextColor;
    colorBufferWrite[write_idx].w = nextAlpha;
    mediumBufferWrite[write_idx].xyz = nextMediumColor;
}