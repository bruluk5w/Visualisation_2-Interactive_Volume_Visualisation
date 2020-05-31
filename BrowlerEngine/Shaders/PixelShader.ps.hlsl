struct PSIn
{
	float4 Color: COLOR;
};

float4 main(PSIn IN) : SV_Target
{
	return IN.Color;
}