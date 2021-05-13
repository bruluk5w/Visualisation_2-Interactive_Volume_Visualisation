struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

SamplerState colorSampler : register(s0);

RWTexture2D<float4> colorTex : register(u0);


float4 main(PS_INPUT input) : SV_Target
{  
    uint2 texDim;
    colorTex.GetDimensions(texDim.x, texDim.y);

    return colorTex.Load(input.uv * texDim);
}