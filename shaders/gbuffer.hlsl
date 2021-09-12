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
    float3 diffuseColor;
    bool hasDiffuseMap;
    float metallicValue;
    bool hasMetallicMap;
    float roughnessValue;
    bool hasRoughnessMap;
    float aoValue;
    bool hasAoMap;
};

struct VS_Input {
    float3 pos : POS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPos : WORLD_POS;
};

SamplerState s_PointBorder : register(s0);

Texture2D albedoTexture : register(t0);
Texture2D metallicTexture : register(t1);
Texture2D roughnessTexture : register(t2);
Texture2D aoTexture : register(t3);

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = mul(model,float4(input.pos, 1.0f));
    float4x4 VP = mul(projection, view);
    
    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.uv = input.uv;
    output.normal = input.normal;
    output.worldPos = worldPos.xyz;
    return output;
}

struct PS_Output {
    float4 albedo :     SV_Target0;
    float4 normal :     SV_Target1;
    float4 position :   SV_Target2;
};

PS_Output ps_main(VS_Output input)
{
    float3 albedo = hasDiffuseMap ? albedoTexture.Sample(s_PointBorder, input.uv).rgb : diffuseColor;
    float metallic = hasMetallicMap ? metallicTexture.Sample(s_PointBorder, input.uv).r : metallicValue;
    float roughness = hasRoughnessMap ? roughnessTexture.Sample(s_PointBorder, input.uv).r : roughnessValue;
    float ao = hasAoMap ? aoTexture.Sample(s_PointBorder, input.uv).r : aoValue;
    
    PS_Output output;
    output.albedo = float4(albedo, metallic);
    output.normal = float4(input.normal, roughness);
    output.position = float4(input.worldPos, ao);
    return output;
}