
struct ModelViewProjection
{
	matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color    : COLOR;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Position : SV_Position;
	float4 Color    : COLOR;
    float2 uv		: TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
	//VSOut OUT;
	//OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
	//OUT.Color = float4(IN.Color, 1.0f);

	//return OUT;
	
    PS_INPUT output;
    output.Position = mul(ModelViewProjectionCB.MVP, float4(input.Position, 1.f));
    output.Color = input.Color;
    output.uv = input.uv;
    return output;
}