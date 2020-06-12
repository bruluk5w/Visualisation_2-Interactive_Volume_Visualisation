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


float4 main(PS_INPUT input) : SV_Target
{
    //int3 dim;
    //volumeTexture.GetDimensions(dim.x, dim.y, dim.z);
    //float4 out_col = float4(input.uv.x, input.uv.y, 0.f, 1.f);
    
    //float4 out_col = texture0.Load(float4(input.uv * 832, 200, 0));
    float4 out_col = volumeTexture.Sample(volumeSampler, float3(input.uv, 0.5)).r * 4;
    out_col.w = 1;
    DeviceMemoryBarrier();
    return out_col;
}