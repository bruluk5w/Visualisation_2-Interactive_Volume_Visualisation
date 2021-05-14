struct PS_INPUT
{
    float4 pos : SV_Position;
    float3 uvw : TEXCOORD3D;
};

struct Constants
{
    float numSlices;
    float3 deltaSlice;
};

ConstantBuffer<Constants> constants : register(b1);

SamplerState preintegrationSampler : register(s0);
SamplerState volumeSampler : register(s1);

Texture2D refractionIntegTex: register(t0);
Texture2D particleColIntegTex: register(t1);
Texture2D opacityIntegTex: register(t2);
Texture2D mediumIntegTex: register(t3);
Texture3D volumeTexture: register(t4);


float4 main(PS_INPUT input) : SV_Target
{
    int3 a;
    volumeTexture.GetDimensions(a.x, a.y, a.z);
    // this is the the dimension of the volume in the same space as the viewing plane
    const float3 volumeTexDim = ((float3) a);
    
    ////const float pitScale = getScaleForPreintegrationTableScale();
    float accumulated = 0;
    float lastSample = 0;
    float3 sampleOffset = 0;
    const float3 sampleDir = constants.deltaSlice;
    for (float i = 0; i < constants.numSlices; ++i)
    {
        const float3 uvw = ((input.uvw + 0.5f * volumeTexDim) + sampleOffset) / volumeTexDim;
        // We need to rescale the values sampled from the volume because we assume only 12 bits out of 16 are used
        // Since our volume texture is normalized to 16bit we would else only use 1/4 of our normalized range
        const float newSample = volumeTexture.Sample(volumeSampler, uvw).r * 4.f;
        accumulated += opacityIntegTex.Sample(preintegrationSampler, float2(lastSample, newSample)).r * length(constants.deltaSlice);
        lastSample = newSample;
        
        // next plane
        sampleOffset += constants.deltaSlice;
    }
    
    return float4(accumulated, accumulated, accumulated, 0.f);

}