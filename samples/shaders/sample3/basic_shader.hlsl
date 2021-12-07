
// When using multiple vertex buffers we can use just one attribute per slot

struct VS_Input
{
    float2 position : POSITION; // This is first declared so it should be in slot 0
    float3 color : VERT_COLOR; // This is second declared so it should be in slot 1
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
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
    return float4(input.color, 1.0f);
}