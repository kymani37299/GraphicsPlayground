cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);
SamplerState s_LinearClamp : register(s2);

Texture2D heightMap : register(t0);
Texture2D terrainTexture : register(t1);

struct VS_Input
{
    float3 pos : POS;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = float4(input.pos, 1.0f);
    float4x4 VP = mul(projection, view);
    
    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.pos.y += 1000.0 * heightMap.SampleLevel(s_LinearBorder, input.uv, 0).r;
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return terrainTexture.Sample(s_LinearClamp, input.uv);
}