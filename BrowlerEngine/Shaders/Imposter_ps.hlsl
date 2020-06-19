struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

SamplerState colorSampler : register(s0);

Texture2D<float4> colorTex : register(t0);


float4 main(PS_INPUT input) : SV_Target
{  
    //if (input.uv.x < 0 || input.uv.x > 1 || input.uv.y < 0 || input.uv.y > 1)
    //    return float4(0, 0, 0, 1);
    //else
    //    return float4(input.uv, 1, 1);
    return colorTex.Sample(colorSampler, input.uv.x)*100;
}