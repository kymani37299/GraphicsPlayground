cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float3 cameraPosition;
    float4x4 viewInv;
    float4x4 projInv;
};

cbuffer Environment : register(b1)
{
    float3 directionalLight;
    uint numPointLights;
};

cbuffer SunCamera : register(b2)
{
    float4x4 sunView;
    float4x4 sunProjection;
    float3 sunPosition;
    float4x4 sunViewInv;
    float4x4 sunProjectionInv;
}

struct PointLight
{
    float3 position;
    float3 color;
};

SamplerState s_PointBorder : register(s0);

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D positionTexture : register(t2);
StructuredBuffer<PointLight> pointLights : register(t3);
Texture2D shadowDepth : register(t4);
Texture2D depthTexture : register(t5);

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
    output.pos =  float4(input.pos,0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}

float attenuate(float dist)
{
    const float3 attenuation = float3(0.6, 2.0, 0.5);
    return 1.0 / (attenuation.x + (attenuation.y * dist) + (attenuation.z * dist * dist));
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    const float PI = 3.1415;
    
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

struct MatInput
{
    float3 position;
    float3 albedo;
    float3 normal;
    float metallic;
    float roughness;
    float ao;
};

float3 PBR_LightEffect(float3 lightDir, float lightDist, float3 lightColor, float3 cameraDir, MatInput mat, float3 F0)
{
    const float PI = 3.1415;
    
    float3 N = mat.normal;
    float3 V = cameraDir;
    
    // calculate per-light radiance
    float3 L = lightDir;
    float3 H = normalize(V + L);
    float distance = lightDist;
    float attenuation = 1.0 / (distance * distance);
    float3 radiance = lightColor * attenuation;
        
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, mat.roughness);
    float G = GeometrySmith(N, V, L, mat.roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - mat.metallic;
        
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    float3 specular = numerator / max(denominator, 0.001);
            
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * mat.albedo / PI + specular) * radiance * NdotL;
}

float3 DepthToPos(float2 uv, float depth)
{
    float3 screenPosition = float3(uv, 1.0 - depth);
    float4 clipSpacePosition = float4(2.0 * screenPosition - 1.0, 1.0);
    float4 viewSpacePosition = mul(projInv, clipSpacePosition);

    // Perspective division
    viewSpacePosition /= viewSpacePosition.w;
    viewSpacePosition.w = 1.0;
    
    float4 worldSpacePosition = mul(viewInv, viewSpacePosition);
    
    return clipSpacePosition.xyz;
    //return worldSpacePosition.xyz;

}

float4 PBR(float2 uv)
{
    MatInput matInput;
    matInput.position = positionTexture.Sample(s_PointBorder, uv).rgb;
    matInput.albedo = albedoTexture.Sample(s_PointBorder, uv).rgb;
    matInput.normal = normalize(normalTexture.Sample(s_PointBorder, uv).rgb);
    matInput.metallic = albedoTexture.Sample(s_PointBorder, uv).a;
    matInput.roughness = normalTexture.Sample(s_PointBorder, uv).a;
    matInput.ao = positionTexture.Sample(s_PointBorder, uv).a;
    
    float3 cameraDir = normalize(cameraPosition - matInput.position);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, matInput.albedo, matInput.metallic);
	           
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < numPointLights; ++i)
    {
        float3 L = normalize(pointLights[i].position - matInput.position);
        float distance = length(pointLights[i].position - matInput.position);
        Lo += PBR_LightEffect(L,distance, pointLights[i].color, cameraDir, matInput, F0);
    }
    
    Lo += PBR_LightEffect(directionalLight, 2.5, float3(173.0 / 255.0, 216.0 / 255.0, 230.0 / 255.0), cameraDir, matInput, F0);
  
    float3 ambient = float3(0.03, 0.03, 0.03) * matInput.albedo * matInput.ao;
    float3 color = ambient + Lo;
	
    color = color / (color + float3(1.0, 1.0, 1.0));
    float gamma = 1.0 / 2.2;
    color = pow(color, float3(gamma, gamma, gamma));
   
    float3 realPos = DepthToPos(uv, depthTexture.SampleLevel(s_PointBorder, uv, 0).r);
    
    return float4(realPos, 1.0);
    //return float4(matInput.position, 1.0);
    //return float4(abs(realPos - matInput.position), 1.0);
}

float4 ps_main(VS_Output input) : SV_Target
{
    return PBR(input.uv);
}