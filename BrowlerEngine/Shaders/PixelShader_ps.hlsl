struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

SamplerState preintegrationSampler : register(s0);
SamplerState volumeSampler : register(s1);

Texture2D refractionIntegTex: register(t0);
Texture2D particleColIntegTex: register(t1);
Texture2D opacityIntegTex: register(t2);
Texture2D mediumIntegTex: register(t3);
Texture3D volumeTexture: register(t4);

globallycoherent RWTexture2D<float> buffer : register(u0);

float getScaleForPreintegrationTableScale()
{
    int2 pitDim; // we expect all preintegration tables to be square and of the same size by convention
    refractionIntegTex.GetDimensions(pitDim.x, pitDim.y);
    return 1.f / (float)pitDim.x;
}

float4 main(PS_INPUT input) : SV_Target
{
    int3 volumeDim;
    volumeTexture.GetDimensions(volumeDim.x, volumeDim.y, volumeDim.z);

    
    const float pitScale = getScaleForPreintegrationTableScale();
    float accumulated = 0;
    const float numSlices = 100.f;
    float lastSample = 0;
    for (float i = 0; i < 1; i+=1.f/numSlices)
    {
        const float3 uvw = float3(input.uv, i);
        // We need to rescale the values sampled from the volume because we assume only 12 bits out of 16 are used
        // Since our volume texture is normalized to 16bit we would else only use 1/4 of our normalized range
        const float newSample = volumeTexture.Sample(volumeSampler, uvw).r * 4;
        accumulated += refractionIntegTex.Sample(preintegrationSampler, float2(lastSample, newSample)).r;
        lastSample = newSample;
    }
    
    const float pixelPerCm = 100;
    float4 out_col = accumulated * (volumeDim.z / pixelPerCm);
    out_col.w = 1;
    return out_col;
}