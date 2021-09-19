cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);
SamplerState s_LinearClamp : register(s2);
SamplerState s_LinearWrap : register(s3);

cbuffer Model : register(b1)
{
    float4x4 model;
}

cbuffer CBEngineGlobals : register(b2)
{
    float g_ScreenWidth;
    float g_ScreenHeight;
    float g_Time; // seconds
};

struct VS_Input
{
    float3 pos : POS;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float4 clipSpacePos : CLIP_SPACE;
    float4 worldSpacePos : WORLD_SPACE;
    float2 uv : TEXCOORD;
    float3 cameraPos : CAM_POS;
};

Texture2D reflectionTexture : register(t0);
Texture2D refractionTexture : register(t1);
Texture2D dudvTexture : register(t2);

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = mul(model, float4(input.pos, 1.0f));
    float4x4 VP = mul(projection, view);

    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.normal = float3(0.0f, 1.0f, 0.0f);
    output.clipSpacePos = output.pos;
    output.worldSpacePos = worldPos;
    output.uv = input.uv;
    output.cameraPos = cameraPosition;
    return output;
}

float2 GetDuDv(float2 uv, float time)
{
    float2 dudvUV = uv;
    dudvUV.x += time;
    dudvUV *= 100.0f;
    dudvUV = frac(dudvUV);
    float2 dudv1 = dudvTexture.Sample(s_LinearWrap, dudvUV).rg;
    
    dudvUV = uv;
    dudvUV.xy += time;
    dudvUV *= 100.0f;
    dudvUV = frac(dudvUV);
    float2 dudv2 = dudvTexture.Sample(s_LinearWrap, dudvUV).rg;

    return (dudv1 + dudv2) / 2.0f;
}

float4 ps_main(VS_Output input) : SV_Target
{
    // Should be variable
    float waterReflectivness = 0.5f;
    float waterDisplacement = 0.02;
    float time = g_Time / 1000.0f;

    float3 cameraDir = normalize(input.cameraPos - input.worldSpacePos);
    float3 refractionCoeff = max(dot(cameraDir, input.normal), 0.0);
    refractionCoeff = pow(refractionCoeff, waterReflectivness);

    float2 screenUV = input.clipSpacePos.xy / input.clipSpacePos.w;
    screenUV = 0.5f * screenUV + 0.5f;
    screenUV.y = 1.0 - screenUV.y;

    float2 dudv = GetDuDv(input.uv, time);

    float2 reflectionUV = screenUV;
    reflectionUV.y = 1.0 - reflectionUV.y;
    reflectionUV += waterDisplacement * dudv;

    float2 refractionUV = screenUV;
    refractionUV += waterDisplacement * dudv;

    float3 refraction = refractionTexture.Sample(s_LinearClamp, refractionUV).rgb;
    screenUV.y = 1.0 - screenUV.y;
    float3 reflection = reflectionTexture.Sample(s_LinearClamp, reflectionUV).rgb;
    float3 waterColor = float3(0.0f, 0.2f, 0.83f);

    float3 waterPlaneColor = lerp(reflection, refraction, refractionCoeff);
    waterPlaneColor = lerp(waterPlaneColor, waterColor, 0.3f);

    return float4(waterPlaneColor, 1.0f);
}
