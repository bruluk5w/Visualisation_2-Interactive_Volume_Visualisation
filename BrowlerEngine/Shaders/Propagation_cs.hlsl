cbuffer constants : register(b0)
{
    float3 bboxmin;
    float3 bboxmax;
    float sliceWidth;
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

[numthreads(16, 16, 1)]

	void main
	(
	uint3 DTid : SV_DispatchThreadID)
    {
		const uint3 read_idx = int3(DTid.xy, 0);
		const uint2 write_idx = DTid.xy;
    
        // viewing ray propagation
		const float3 viewingRayPositionOld = viewingRayPositionBufferRead.Load(read_idx).xyz;
		const float3 viewingRayDirectionOld = viewingRayDirectionBufferRead.Load(read_idx).xyz;

        // Advance viewing ray in world space
		const float3 viewingRayPositionNew = viewingRayPositionOld + viewingRayDirectionOld;
	    const float3 viewingRayDirectionNew = viewingRayDirectionOld; // todo
    
        // TODO: Add refraction here
    
		viewingRayPositionBufferWrite[write_idx].xyz = viewingRayPositionNew;
	    viewingRayDirectionBufferWrite[write_idx].xyz = viewingRayDirectionNew; // has to be normalized

        // Sampling the volume
        // We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/4 of the normalized range is utilized.
        const float rawScalarSampleOld = volumeTexture.SampleLevel(volumeSampler, getUVCoordinates(viewingRayPositionOld), 0) * 8;


	colorBufferWrite[write_idx].xyz = ((float3) DTid) / 1024.0f;
		colorBufferWrite[write_idx].w = 1;
	}