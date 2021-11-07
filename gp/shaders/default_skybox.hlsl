CB_CAMERA(0);

TextureCube skybox : register(t0);

struct VS_Input
{
    float3 pos : POS;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 skyRay : SKY_RAY;
};

VS_Output vs_main(VS_Input input)
{
    float4x4 viewNoTranslation = view;
    viewNoTranslation[0].w = 0.0;
    viewNoTranslation[1].w = 0.0;
    viewNoTranslation[2].w = 0.0;
    float4 position = mul(mul(projection, viewNoTranslation), float4(input.pos, 1.0));

    VS_Output output;
    output.pos = position;
    output.skyRay = input.pos;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return skybox.Sample(s_PointBorder, input.skyRay);
}