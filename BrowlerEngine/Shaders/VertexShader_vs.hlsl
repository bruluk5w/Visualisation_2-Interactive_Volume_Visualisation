
struct Constants
{
    matrix ModelMatrix;
    matrix ViewProjection;
    float voxelsPerCm;
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
	//VSOut OUT;
	//OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
	//OUT.Color = float4(IN.Color, 1.0f);

	//return OUT;
	
    PS_INPUT output;
    float4 worldSpacePos = mul(constants.ModelMatrix, float4(input.Position, 1.f));
    output.uvw = worldSpacePos * constants.voxelsPerCm;
    output.Position = mul(constants.ViewProjection, worldSpacePos);

    return output;
}