
struct Constants
{
    matrix modelviewProjection;
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
    output.uv = input.uv;
    output.Position = mul(constants.modelviewProjection, float4(input.Position, 1.f));
    return output;
}


/*/TODO: DELETE THIS

static const uint bufferWidth = 10; // in pixel

const int3 read_idx = int3(DTid.xy + bufferWidth, 0);
const uint2 write_idx = DTid.xy + bufferWidth;
// light propagation
const float3 lightOld = lightBufferRead.Load(read_idx).xyz;
const float3 lightDirectionOld = lightDirectionBufferRead.Load(read_idx).xyz;
// intersect with previous light plane
const float3 lightNew = lightOld;
lightBufferWrite[write_idx].xyz = lightNew;

const float3 lightDirectionNew = lightDirectionOld;
lightDirectionBufferWrite[write_idx].xyz = lightDirectionNew;


// viewing ray propagation
const float3 viewingRayPositionOld = viewingRayPositionBufferRead.Load(read_idx).xyz;
const float3 viewingRayDirectionOld = viewingRayDirectionBufferRead.Load(read_idx).xyz;
const float4 colorOld = colorBufferRead.Load(read_idx);
const float4 colorNew = colorOld;

const float3 mediumOld = mediumBufferRead.Load(read_idx).xyz;
const float3 mediumNew = mediumOld;
mediumBufferWrite[write_idx].xyz = mediumNew;

//Advance viewing ray in world space
const float3 viewingRayPositionNew = viewingRayPositionOld + viewingRayDirectionOld * sliceWidth;
const float3 viewingRayDirectionNew = viewingRayDirectionOld; // todo

viewingRayPositionBufferWrite[write_idx].xyz = viewingRayPositionNew;
viewingRayDirectionBufferWrite[write_idx].xyz = viewingRayDirectionNew; // has to be normalized

// Sampling the volume
// We need to multiply the values sampled from the volume because we assume only 12 bits out of 16 are used. Else only use 1/4 of the normalized range is utilized.
const float rawScalarSampleOld = volumeTexture.SampleLevel(volumeSampler, (viewingRayPositionOld * worldSpaceToNormalizedVolume) + 0.5f, 0) * 4;
const float rawScalarSampleNew = volumeTexture.SampleLevel(volumeSampler, (viewingRayPositionNew * worldSpaceToNormalizedVolume) + 0.5f, 0) * 4;
const float refractionContrib = refractionIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
const float opacityContrib = opacityIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
// todo: change to rgb:
const float particleColorContrib = particleColIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;
const float mediumContrib = mediumIntegTex.SampleLevel(preintegrationSampler, float2(rawScalarSampleOld, rawScalarSampleNew), 0).r;

colorBufferWrite[write_idx] = colorNew + rawScalarSampleNew;
colorBufferWrite[write_idx].w = 1;
*/