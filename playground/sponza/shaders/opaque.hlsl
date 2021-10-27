CB_CAMERA(0);

struct VS_Input
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

SamplerState diffuseSampler : register(s0);
Texture2D diffuseTexture : register(t0);

VS_Output vs_main(VS_Input input)
{
    float4x4 VP = mul(projection, view);
    float4 worldPos = float4(input.position, 1.0f);

    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.uv = input.uv;
    output.normal = input.normal;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return diffuseTexture.Sample(diffuseSampler, input.uv);
}