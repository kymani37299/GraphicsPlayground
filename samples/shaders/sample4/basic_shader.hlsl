
struct VS_Input
{
    float2 position : POSITION;
    float3 color : VERT_COLOR;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer ColorOverride : register(b0) // We are using slot 0 for this buffer
{
    // You can name the variables whatever you want
    // The order of variables must match the order in code
    // So in this case first goes bool for enabled and then float3(Vec3) for color
    bool ColorOverride_Enabled; 
    float3 ColorOverride_Color;
};

// vs_main : Entry point of the vertex shader
VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}

// ps_main : Entry point of the pixel shader
float4 ps_main(VS_Output input) : SV_Target
{
    if (ColorOverride_Enabled)
    {
        return float4(ColorOverride_Color, 1.0f);
    }
    else
    {
        return float4(input.color, 1.0f);
    }
}