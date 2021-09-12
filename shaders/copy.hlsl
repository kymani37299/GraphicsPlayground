cbuffer CopyParams : register(b0)
{
    float4 rect; // Top left, bottom right
};

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
    float uvx = input.uv.x;
    float uvy = input.uv.y;
    
    float2 tl = rect.xy; // Top left
    float2 dr = rect.zw; // Down right
    
    if(uvx < tl.x || uvx > dr.x || uvy < tl.y || uvy > dr.y)
        discard;
    
    float2 texUV = input.uv - tl;
    texUV /= (dr - tl);
    
    float3 texColor = inputTexture.Sample(s_PointBorder, texUV).rgb;
    
    return float4(texColor, 1.0);
}