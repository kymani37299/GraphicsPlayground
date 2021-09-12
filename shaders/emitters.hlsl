cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

cbuffer Instance : register(b2)
{
    float4x4 model;
}

cbuffer Material : register(b3)
{
    float3 diffuse;
    bool hasDiffuseMap;
};

struct VS_Input
{
    float3 pos : POS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    float4x4 VP = mul(projection, view);
    
    VS_Output output;
    output.pos = mul(VP, worldPos);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    const float emitterStrength = 0.6;
    return clamp(float4(emitterStrength * float3(1.0, 1.0, 1.0) + diffuse, 1.0),0.0, 1.0);
}