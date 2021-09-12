SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);
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
    SamplerState blurSampler = s_LinearBorder;
    
    const float directions = 16.0; // BLUR DIRECTIONS (Default 16.0 - More is better but slower)
    const float quality = 3.0; // BLUR QUALITY (Default 3.0 - More is better but slower)
    const float size = 8.0; // BLUR SIZE (Radius)
    const float PI2 = 6.28318530718;
    
    float2 screen_size = float2(1024, 768); // TODO: This should not be hardcoded
    float2 radius = size / screen_size;
    float4 color = inputTexture.Sample(blurSampler, input.uv);
    
    for (float d = 0.0; d < PI2; d += PI2 / directions)
    {
        for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
        {
            float2 _uv = input.uv + float2(cos(d), sin(d)) * radius * i;
            color += inputTexture.Sample(blurSampler, _uv);
        }
    }
    
    color /= quality * directions - 15.0;
    return color;
}