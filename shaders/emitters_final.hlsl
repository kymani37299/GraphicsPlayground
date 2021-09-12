SamplerState s_PointBorder : register(s0);
Texture2D inputTexture : register(t0);

struct VS_Input
{
    float2 pos : POS;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float4 finalColor = inputTexture.Sample(s_PointBorder, input.uv);
    if (finalColor.a < 0.01)
        discard;
    return finalColor;
}