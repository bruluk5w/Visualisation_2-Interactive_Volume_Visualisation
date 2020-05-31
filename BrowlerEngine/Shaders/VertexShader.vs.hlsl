//#define RootSig \
	//"RootFlags( " \
	//"	ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | " \
	//"	ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | " \
	//"	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | " \
	//"	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | " \
	//"	D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS), " \
	//"RootConstants(num32BitConstants=16, b0, 0, SHADER_VISIBILITY_VERTEX) " // MVP
	// Some examples:
	//"CBV(b0, space = 1, flags = DATA_STATIC), " \
	//"SRV(t0), " \
	//"UAV(u0), " \
	//"DescriptorTable( CBV(b1), " \
	//"	SRV(t1, numDescriptors = 8, " \
	//"	flags = DESCRIPTORS_VOLATILE), " \
	//"	UAV(u1, numDescriptors = unbounded, " \
	//"	flags = DESCRIPTORS_VOLATILE)), " \
	//"DescriptorTable(Sampler(s0, space=1, numDescriptors = 4)), " \
	//"StaticSampler(s1)," \
	//"StaticSampler(s2, " \
	//"addressU = TEXTURE_ADDRESS_CLAMP, " \
	//"filter = FILTER_MIN_MAG_MIP_LINEAR )"
	
struct ModelViewProjection
{
	matrix MVP;
};

ConstantBuffer<ModelViewProjection> ModelViewProjectionCB : register(b0);

struct VertexPosColor
{
	float3 Position : POSITION;
	float3 Color    : COLOR;
};

struct VSOut
{
	float4 Color    : COLOR;
	float4 Position : SV_Position;
};

//[RootSignature(RootSig)]
VSOut main(VertexPosColor IN)
{
	VSOut OUT;
	OUT.Position = mul(ModelViewProjectionCB.MVP, float4(IN.Position, 1.0f));
	OUT.Color = float4(IN.Color, 1.0f);

	return OUT;
}