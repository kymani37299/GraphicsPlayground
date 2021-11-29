// RasterizerState: BACKFACE_CCW
// DepthState: ENABLED

// TODO: Sepparate variation for USE_ALPHA_BLEND

CB_CAMERA(0);
CB_MODEL(1);

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
    float3 worldPos : WORLD_POS;
};

SamplerState diffuseSampler : register(s0);
Texture2D diffuseTexture : register(t0);

VS_Output vs_main(VS_Input input)
{
    float4x4 MVP = mul(mul(projection, view), model);
    float4 worldPos = float4(input.position, 1.0f);

    VS_Output output;
    output.pos = mul(MVP, worldPos);
    output.uv = input.uv;
    output.normal = input.normal;
    output.worldPos = worldPos;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    // Constants
    const float3 sunDir = normalize(float3(-0.7, 0.5, -1.2));
    const float3 sunColor = float3(1.0, 1.0, 1.0);
    const float3 sunPos = float3(0.0f, 0.0f, 0.0f);
    const float3 pointLightPos = float3(0.0f, 0.0f, 0.0f);
    const float3 pointLightColor = float3(1.0f, 1.0f, 1.0f);
    const float ambientFactor = 0.1;

    const float4 objectColor = diffuseTexture.Sample(diffuseSampler, input.uv);
    if (objectColor.a < 0.01f) discard;

    const float3 N = normalize(input.normal);
    const float3 L = normalize(sunPos - input.worldPos);
    const float3 V = normalize(cameraPosition - input.worldPos);
    const float3 R = reflect(-V, N);

    float3 ambient = ambientFactor * sunColor;

    float diffuseFactor = max(dot(N, L), 0.0);
    float3 diffuse = diffuseFactor * sunColor;

    float specularFactor = pow(max(dot(V, R), 0.0), 32);
    float3 specular = 0.5f * specularFactor * sunColor;

    float3 finalColor = (diffuse + ambient + specular) * objectColor.rgb;

#ifdef USE_ALPHA_BLEND
    return float4(finalColor, objectColor.a);
#else
    return float4(finalColor, 1.0f);
#endif // USE_ALPHA_BLEND
}