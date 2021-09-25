CB_CAMERA(0);

cbuffer GeometryParams : register(b1)
{
    float4 clipPlane;
    bool useClipping;
};

Texture2D heightMap : register(t0);
Texture2D terrainTexture : register(t1);

struct VS_Input
{
    float3 pos : POS;
    float2 uv : TEXCOORD;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float clip : SV_ClipDistance0;
};

VS_Output vs_main(VS_Input input)
{
    float terrainHeight = 150.0 * heightMap.SampleLevel(s_LinearBorder, input.uv, 0).r;
    terrainHeight = pow(terrainHeight, 1.3);
    float4x4 VP = mul(projection, view);

    float4 worldPos = float4(input.pos, 1.0f);
    worldPos.y += terrainHeight;

    VS_Output output;
    output.pos = mul(VP, worldPos);
    output.uv = input.uv;
    output.clip = useClipping ? dot(worldPos, clipPlane) : 1.0f;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    // TODO: Make this uniform
    float uvMul = 20.0f;

    float2 textureUV = input.uv;
    textureUV *= uvMul;
    textureUV -= floor(textureUV);

    return terrainTexture.Sample(s_LinearClamp, textureUV);
}