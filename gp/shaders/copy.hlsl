struct VS_Input
{
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = float4(input.position, 0.0, 1.0);
    output.uv = input.uv;
    return output;
}

Texture2D tex_source : register(t0);

float4 ps_main(VS_Output input) : SV_Target
{
    return tex_source.Sample(s_LinearBorder, input.uv);
}