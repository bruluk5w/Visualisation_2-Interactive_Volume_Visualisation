
struct Constants
{
    matrix modelviewProjection;
    float2 uvOffset;
    float2 uvRangeScale;
};

ConstantBuffer<Constants> constants : register(b0);

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 uv : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    output.uv = input.uv * constants.uvRangeScale + constants.uvOffset;
    output.Position = mul(constants.modelviewProjection, float4(input.Position, 1.f));
    return output;
}