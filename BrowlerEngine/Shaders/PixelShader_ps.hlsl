struct PS_INPUT
{
    float4 pos : SV_Position;
    float3 uvw : TEXCOORD3D;
};

struct Constants
{
    float voxelsPerCm;
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

globallycoherent RWTexture2D<float> buffer : register(u0);

//float getScaleForPreintegrationTableScale()
//{
//    int2 pitDim; // we expect all preintegration tables to be square and of the same size by convention
//    refractionIntegTex.GetDimensions(pitDim.x, pitDim.y);
//    return 1.f / (float)pitDim.x;
//}

float4 main(PS_INPUT input) : SV_Target
{
    //const float voxelScale = 1.f / constantsCB.voxelsPerCm;
    int3 volumeDim;
    volumeTexture.GetDimensions(volumeDim.x, volumeDim.y, volumeDim.z);
    float3 volumeDimension = (float3) volumeDim;
    //const float3 scaledVolume = ((float3) volumeDim) * voxelScale;
    
    ////const float pitScale = getScaleForPreintegrationTableScale();
    float accumulated = 0;
    float lastSample = 0;
    float3 sampleOffset = 0;
    for (float i = 0; i < constants.numSlices; ++i)
    {
        // We need to rescale the values sampled from the volume because we assume only 12 bits out of 16 are used
        // Since our volume texture is normalized to 16bit we would else only use 1/4 of our normalized range
        const float newSample = volumeTexture.Sample(volumeSampler, (input.uvw + 0.5f * volumeDimension) / volumeDimension + sampleOffset).r * 4.f;
        accumulated += newSample;
        //accumulated += refractionIntegTex.Sample(preintegrationSampler, float2(lastSample, newSample)).r;
        lastSample = newSample;
        
        // next plane
        sampleOffset += constants.deltaSlice;
    }
    
    //float4 out_col = accumulated * (volumeDim.z / constantsCB.voxelsPerCm);
    float4 out_col = accumulated;
    out_col.w = 1;
    return out_col;
}