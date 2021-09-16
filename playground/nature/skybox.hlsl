cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);

struct VS_Input
{
    float3 pos : POS;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
};

VS_Output vs_main(VS_Input input)
{
    float4 worldPos = float4(input.pos, 1.0f);
    float4x4 VP = mul(projection, view);
    
    VS_Output output;
    output.pos = mul(VP, worldPos);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return float4(1.0,0.0,0.0,1.0);

}