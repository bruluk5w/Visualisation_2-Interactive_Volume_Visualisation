struct PS_INPUT
{
    float4 pos : SV_Position;
};

float4 main(PS_INPUT input) : SV_Target
{
    return 1;
}