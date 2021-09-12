cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

SamplerState s_PointBorder : register(s0);
Texture2D skybox : register(t0);

struct VS_Input
{
    float3 pos : POS;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 localPos : LOCAL_POS;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(mul(projection, view), float4(input.pos, 1.0));
    output.localPos = input.pos;
    return output;
}

float2 SampleSphericalMap(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}


float4 ps_main(VS_Output input) : SV_Target
{
    float2 uv = SampleSphericalMap(normalize(input.localPos));
    return skybox.Sample(s_PointBorder, uv);
}