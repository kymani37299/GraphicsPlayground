cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

cbuffer Instance : register(b1)
{
    float4x4 model;
};

SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);

Texture2D albedoTexture : register(t0);

struct VS_Input
{
    float3 pos : POS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POS;
};

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    float4x4 VP = mul(projection, view);
    
    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.uv = input.uv;
    output.normal = input.normal;
    output.worldPos = worldPos.xyz;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return albedoTexture.Sample(s_PointBorder, input.uv);

}