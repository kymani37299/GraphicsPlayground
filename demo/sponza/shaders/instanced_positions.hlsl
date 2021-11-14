CB_CAMERA(0);
CB_MODEL(1);

struct VS_Input
{
    float3 position : POSITION;
    float3 positionOffset : I_POSITION_OFFSET; // I_ prefix will automatically set this attribute as per instance
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float4 color : VERT_COLOR;
};

VS_Output vs_main(VS_Input input)
{
    //float4x4 MVP = mul(mul(projection, view), model);
    float4x4 MVP = mul(projection, view);
    float4 worldPos = float4(input.position, 1.0f);

    VS_Output output;
    output.pos = float4(input.positionOffset, 0.0) + mul(MVP, worldPos);
    output.color = float4(1.0, 1.0, 0.0, 1.0);
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return input.color;
}