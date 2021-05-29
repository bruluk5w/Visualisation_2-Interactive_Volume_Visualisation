struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

SamplerState colorSampler : register(s0);

Texture2D<float4> colorTex : register(t0);


float4 main(PS_INPUT input) : SV_Target
{  
    return colorTex.SampleLevel(colorSampler, input.uv, 0);
}