
struct Constants
{
    matrix modelMatrix;
    matrix viewProjection;
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
    float4 uvw : TEXCOORD3D;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldSpacePos = mul(constants.modelMatrix, float4(input.Position, 1.f));
    output.uvw = worldSpacePos;
    output.Position = mul(constants.viewProjection, worldSpacePos);

    return output;
}