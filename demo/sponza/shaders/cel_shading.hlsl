CB_CAMERA(0);
CB_MODEL(1);

struct VS_Input
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_Output vs_main(VS_Input input, uint instanceID : SV_InstanceID)
{
    const float4x4 MVP = mul(mul(projection, view), model);
    const float4 worldPos = float4(input.position, 1.0f);

    VS_Output output;
    output.position = mul(MVP, worldPos);
    output.normal = mul(model, float4(input.normal, 0.0)).xyz;
    output.uv = input.uv;
    return output;
}

SamplerState AnisotropicWrap : register(s0);
Texture2D<float4> tex_albedo : register(t0);

float4 ps_main(VS_Output input) : SV_Target
{
    const float3 dirLight = normalize(float3(1.0, -1.5, -1.0));
    
    float4 albedo = tex_albedo.Sample(AnisotropicWrap, input.uv);
    if(albedo.a < 0.01f) discard;
    
    float ndl = dot(dirLight, input.normal);
    float shade = 0.4f + 0.4f * step(ndl, 0.5f) + 0.2 * step(ndl, 0.8f);
    return shade * albedo;
}