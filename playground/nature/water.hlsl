cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);
SamplerState s_LinearClamp : register(s2);

cbuffer Model : register(b1)
{
    float4x4 model;
}

struct VS_Input
{
    float3 pos : POS;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float4 clipSpace : CLIP_SPACE;
    float2 uv : TEXCOORD;
};

Texture2D reflectionTexture : register(t0);
Texture2D refractionTexture : register(t1);

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    float4x4 VP = mul(projection, view);

    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.clipSpace = output.pos;
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float2 screenUV = input.clipSpace.xy / input.clipSpace.w;
    screenUV = 0.5f * screenUV + 0.5f;
    screenUV.y = 1.0 - screenUV.y;

    float3 refraction = refractionTexture.Sample(s_LinearClamp, screenUV).rgb;
    screenUV.y = 1.0 - screenUV.y;
    float3 reflection = reflectionTexture.Sample(s_LinearClamp, screenUV).rgb;
    float3 waterColor = float3(0.0f, 0.2f, 0.83f);

    float3 waterPlaneColor = lerp(refraction, reflection, 0.5f);
    waterPlaneColor = lerp(waterPlaneColor, waterColor, 0.2f);

    return float4(waterPlaneColor, 1.0f);
}
