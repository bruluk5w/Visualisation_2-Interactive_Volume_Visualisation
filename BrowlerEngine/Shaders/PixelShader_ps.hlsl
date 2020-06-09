struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

SamplerState sampler0 : register(s0);
Texture3D texture0 : register(t0);

float4 main(PS_INPUT input) : SV_Target
{
    int3 dim;
    texture0.GetDimensions(dim.x, dim.y, dim.z);
    float4 out_col = float4(input.uv.x, input.uv.x, 0.f, 1.f); //texture0.Sample(sampler0, float3(input.uv, dim.z * 0.5));
    return out_col;
}