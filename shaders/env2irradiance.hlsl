cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
};

SamplerState s_PointBorder : register(s0);
TextureCube envMap : register(t0);

struct VS_Input
{
    float3 pos : POS;
};

struct VS_Output
{
    float4 pos : SV_POSITION;
    float3 localPos : LOCAL_POS;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = mul(mul(projection, view), float4(input.pos, 1.0));
    output.localPos = input.pos;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    const float PI = 3.14159265359;
    
    // the sample direction equals the hemisphere's orientation 
    float3 normal = normalize(input.localPos);
  
    float3 irradiance = float3(0.0, 0.0, 0.0);
  
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += envMap.Sample(s_PointBorder, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
  
    return float4(irradiance, 1.0);
}