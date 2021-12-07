
struct VS_Input
{
    float2 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Texture2D quadTexture : register(t0); // Bind it to slot 0

// vs_main : Entry point of the vertex shader
VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.position = float4(input.position, 0.0, 1.0);
    output.texcoord = input.texcoord;
    return output;
}

// Explaination for s_LinearClamp
// In HLSL when we sample the texture we must specify the sampler.
// Sampler specifies how are we reading the values in the textures
// 
// In GP there are some static samplers we could use
//  s_PointBorder - Take closest pixel, if out of range use black color
//  s_LinearBorder - Interpolates between neighbour pixels, if out of range use black color
//  s_LinearClamp - Interpolates between neighbour pixels, if out of range use closest pixel
//  s_LinearWrap - Interpolates between neighbour pixels, if out of range then repeat the texture again ( texcoords = texcoords % 1 )
// 
// You can also make your own sampler and bind it to pipeline using GP::GfxSampler
// This is shown in sample _ (TODO)

// ps_main : Entry point of the pixel shader
float4 ps_main(VS_Output input) : SV_Target
{
    return quadTexture.Sample(s_LinearClamp, input.texcoord);
}