#include "Common.hlsli"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

cbuffer constants : register(b0)
{
    float3 bboxmin;
    float texDimToUV;
    float3 bboxmax;
    float3 deltaSlice;
    float3 planeRight;
    float3 planeDown;
    float3 topLeft;
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

Texture2D<float> refractionIntegTex : register(t2);
Texture2D<float4> particleColIntegTex : register(t3);
Texture2D<float> opacityIntegTex : register(t4);
Texture2D<float4> mediumIntegTex : register(t5);
Texture3D<float> volumeTexture : register(t6);

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


float3 getUVCoordinatesVolume(float3 worldCoord)
{
	return (worldCoord - bboxmin) / (bboxmax - bboxmin);
}

void sampleLightTextures(float3 worldCoord, out float3 lightDirection, out float3 lightIntensity)
{
    const float3 offset = worldCoord - topLeft;
    const float2 uv = float2(dot(offset, planeRight), dot(offset, planeDown)) * texDimToUV;
    lightDirection = lightDirectionBufferRead.SampleLevel(lightSampler, uv, 0).xyz;
    lightIntensity = lightBufferRead.SampleLevel(lightSampler, uv, 0).xyz;
}


float integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).r;
}

float3 integrationTable(float previousScalarSample, float currentScalarSample, Texture2D<float4> tex)
{
    return tex.SampleLevel(preintegrationSampler, float2(previousScalarSample, currentScalarSample), 0).rgb;
}

bool rayIntersectsVolume(float3 rayPos, float3 rayDir)
{
    float xdistancemax = bboxmax.x - rayPos.x;
    // (xdistancemax * rayDir > 0) checks if they have the same preceding sign (pos/neg)
    bool intersectsPositiveX = (xdistancemax * rayDir.x > 0) && all(abs((rayPos + rayDir * (xdistancemax / rayDir.x)).yz) <= bboxmax.yz);
    float xdistancemin = bboxmin.x - rayPos.x;
    bool intersectsNegativeX = (xdistancemin * rayDir.x > 0) && all(abs((rayPos + rayDir * (xdistancemin / rayDir.x)).yz) <= bboxmax.yz);
    float ydistancemax = bboxmax.y - rayPos.y;
    bool intersectsPositiveY = (ydistancemax * rayDir.y > 0) && all(abs((rayPos + rayDir * (ydistancemax / rayDir.y)).xz) <= bboxmax.xz);
    float ydistancemin = bboxmin.y - rayPos.y;
    bool intersectsNegativeY = (ydistancemin * rayDir.y > 0) && all(abs((rayPos + rayDir * (ydistancemin / rayDir.y)).xz) <= bboxmax.xz);
    float zdistancemax = bboxmax.z - rayPos.z;
    bool intersectsPositiveZ = (zdistancemax * rayDir.z > 0) && all(abs((rayPos + rayDir * (zdistancemax / rayDir.z)).xy) <= bboxmax.xy);
    float zdistancemin = bboxmin.z - rayPos.z;
    bool intersectsNegativeZ = (zdistancemin * rayDir.z > 0) && all(abs((rayPos + rayDir * (zdistancemin / rayDir.z)).xy) <= bboxmax.xy);
    
    return (intersectsNegativeX || intersectsNegativeY || intersectsNegativeZ || 
            intersectsPositiveX || intersectsPositiveY || intersectsPositiveZ);
}

void reconstructGradient(float3 worldPos, out float3 gradient, out float3 refractionGradient)
{
    const float stepLength = 0.5;
    float left =    volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(stepLength, 0, 0)), 0);
    float right =   volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(stepLength, 0, 0)), 0);
    float top =     volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(0, stepLength, 0)), 0);
    float bottom =  volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(0, stepLength, 0)), 0);
    float front =   volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos - float3(0, 0, stepLength)), 0);
    float back =    volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(worldPos + float3(0, 0, stepLength)), 0);
               
    gradient = normalize(float3(left - right, top - bottom, front - back));
    
    uint2 texDim;
    refractionIntegTex.GetDimensions(texDim.x, texDim.y);
    float leftIOR =     integrationTable(left, left + (1.0f / texDim.x), refractionIntegTex);
    float rightIOR =    integrationTable(right, right + (1.0f / texDim.x), refractionIntegTex);
    float topIOR =      integrationTable(top, top + (1.0f / texDim.x), refractionIntegTex);
    float bottomIOR =   integrationTable(bottom, bottom + (1.0f / texDim.x), refractionIntegTex);
    float frontIOR =    integrationTable(front, front + (1.0f / texDim.x), refractionIntegTex);
    float backIOR =     integrationTable(back, back + (1.0f / texDim.x), refractionIntegTex);
    
    refractionGradient = float3(leftIOR - rightIOR, topIOR - bottomIOR, frontIOR - backIOR);

}

float phongSpecular(float3 lightDir, float3 viewingDir, float3 normal, float reflectivity)
{
    if (dot(normal, lightDir) > 0) //Lamberts Cosine Law, see also http://www.cs.toronto.edu/~jacobson/phong-demo/
    {
        lightDir = normalize(lightDir);
        viewingDir = normalize(viewingDir);
        float3 lightReflected = reflect(-lightDir, normal);
        float specularAngle = max(dot(lightReflected, viewingDir), 0.00001);
        return pow(specularAngle, reflectivity);
    }
    else return 0;
}


[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void main ( uint3 DTid : SV_DispatchThreadID )

{
	const uint3 read_idx = int3(DTid.xy, 0);
	const uint2 write_idx = DTid.xy;
    const float sliceDepth = length(deltaSlice);
    
    /*
     *    Light Propagation
     */
    
    {
    
    
    const float3 currentLightDirection = lightDirectionBufferRead.Load(read_idx).xyz;
    const float3 currentLightIntensity = lightBufferRead.Load(read_idx).xyz;
    
    float len = dot(currentLightDirection, normalize(deltaSlice));
    const float2 previousLightPos = (DTid - currentLightDirection * length(deltaSlice) / len).xy;
    
    float3 lightDirection = lightDirectionBufferRead.SampleLevel(lightSampler, previousLightPos * texDimToUV, 0).xyz;
    lightIntensity = lightBufferRead.SampleLevel(lightSampler, previousLightPos * texDimToUV, 0).xyz;
    
    lightDirectionBufferWrite[write_idx].xyz = currentLightDirection;
    lightBufferWrite[write_idx].xyz = currentLightIntensity;
        
    }


    /*
     *   Viewing Ray Propagation
     */
    const float3 currentViewingRayPosition = viewingRayPositionBufferRead.Load(read_idx).xyz;
    const float3 currentViewingRayDirection = viewingRayDirectionBufferRead.Load(read_idx).xyz;
    
    // We know that the write buffer holds the previous pixel's viewing ray position
    const float3 previousViewingRayPosition = viewingRayPositionBufferWrite.Load(read_idx).xyz;
    
    float4 currentColor = colorBufferRead.Load(read_idx);
    
    if (currentColor.w >= 1)
    { // Would be sufficient to do it only once and write the second plane/layer into the next Buffer
        colorBufferWrite[write_idx] = currentColor;
        return; 
    }
    if (!rayIntersectsVolume(currentViewingRayPosition, currentViewingRayDirection))
    {
        colorBufferWrite[write_idx].xyz = currentColor.xyz + checkerBoard(currentViewingRayDirection) * (1 - currentColor.w);
        colorBufferWrite[write_idx].w = 1;
        return;
    }
    
    float3 currentMediumColor = mediumBufferRead.Load(read_idx).xyz;
    
    float3 diffuseLight = lightBufferRead.Load(read_idx).xyz; // Need to calculate values first
    // Sampling light information
    float3 lightDirection;
    float3 lightIntensity;
    sampleLightTextures(currentViewingRayPosition, lightDirection, lightIntensity);
    // reconstruct gradient of scalar field
    float3 gradient;
    float3 refractionGradient;
    reconstructGradient(currentViewingRayPosition, gradient, refractionGradient);
    //float3 specularLight = specularBDRF(currentLightDirection, currentViewingRayDirection, delta_f);
    
    
    

    // Sampling the volume
    // We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/8 of the normalized range is utilized.
    const float previousScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(previousViewingRayPosition), 0) * 8;
    const float currentScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(currentViewingRayPosition), 0) * 8;
    
    float currentIndexOfRefraction = integrationTable(previousScalarSample, currentScalarSample, refractionIntegTex);
    
    // Advance viewing ray in world space
    float len = dot(currentViewingRayDirection, normalize(deltaSlice));
    const float3 nextViewingRayPosition = currentViewingRayPosition + currentViewingRayDirection * length(deltaSlice) / len;
    const float3 nextViewingRayDirection = currentViewingRayDirection + sliceDepth * refractionGradient;
    
    const float nextScalarSample = volumeTexture.SampleLevel(volumeSampler, getUVCoordinatesVolume(nextViewingRayPosition), 0) * 8;
    
    float3 color = integrationTable(previousScalarSample, currentScalarSample, particleColIntegTex);
    float alpha = integrationTable(previousScalarSample, currentScalarSample, opacityIntegTex);
    float3 medium = integrationTable(previousScalarSample, currentScalarSample, mediumIntegTex);
    
    float nextIndexOfRefraction = integrationTable(currentScalarSample, nextScalarSample, refractionIntegTex);
    // the lower reflectivity the more it reflects, therefore the higher the difference, the more is subtracted
    float reflectivity = 20 - 50 * abs(nextIndexOfRefraction - currentIndexOfRefraction);
    float3 specularLight = phongSpecular(lightDirection, currentViewingRayDirection, gradient, reflectivity);
    
    float3 nextColor = currentColor.xyz + (1 - currentColor.w) * currentMediumColor * (alpha * color * diffuseLight + specularLight);
    float nextAlpha = currentColor.w + (1 - currentColor.w) * alpha.x;
    float3 nextMediumColor = currentMediumColor * (1 - medium);
    
   
    
    viewingRayPositionBufferWrite[write_idx].xyz = nextViewingRayPosition;
    viewingRayDirectionBufferWrite[write_idx].xyz = nextViewingRayDirection; // has to be normalized
    colorBufferWrite[write_idx].xyz = nextColor;
    colorBufferWrite[write_idx].w = nextAlpha;
    mediumBufferWrite[write_idx].xyz = nextMediumColor;
}