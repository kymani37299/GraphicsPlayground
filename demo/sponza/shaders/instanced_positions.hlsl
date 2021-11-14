CB_CAMERA(0);

#ifdef USE_MODEL
CB_MODEL(1);
#endif

struct VS_Input
{
    float3 position : POSITION;
    float3 positionOffset : I_POSITION_OFFSET; // I_ prefix will automatically set this attribute as per instance
    // TODO: Add SV_InstanceID
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float4 color : VERT_COLOR;
};

VS_Output vs_main(VS_Input input)
{
#ifdef USE_MODEL
    const float4x4 MVP = mul(mul(projection, view), model);
    const float4 worldPos = float4(input.position, 1.0f) + float4(input.positionOffset, 0.0) / float4(model[0].x, model[1].y, model[2].z, 1.0f);
#else
    const float4x4 MVP = mul(projection, view);
    const float4 worldPos = float4(4.0f * input.position, 1.0f) + float4(input.positionOffset, 0.0);
#endif

    VS_Output output;
    output.pos = mul(MVP, worldPos);

//#ifdef USE_INSTANCE_ID
//    output.color = float4(input.instanceID / 255, (255 - input.instanceID) / 255, input.instanceID * 2 / 255, 1.0);
//#else
#ifdef USE_MODEL
    output.color = float4(1.0, 0.0, 1.0, 0.5);
#else
    output.color = float4(0.0, 1.0, 1.0, 1.0);
#endif
//#endif 
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    return input.color;
}