// Shaders are written in HLSL like you would write it for any other program
// By default shaders stages that are used are VertexShader(entry point vs_main) and PixelShader(entry point ps_main)

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