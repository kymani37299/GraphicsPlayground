#define CB_CAMERA(x) cbuffer Camera : register(b##x) \
    { \
        float4x4 view; \
        float4x4 projection; \
        float3 cameraPosition; \
    }

#define CB_ENGINE_GLOBALS(x) cbuffer CBEngineGlobals : register(b##x) \
	{ \
	    float g_ScreenWidth; \
	    float g_ScreenHeight; \
	    float g_Time; \
	}

SamplerState s_PointBorder : register(s0);
SamplerState s_LinearBorder : register(s1);
SamplerState s_LinearClamp : register(s2);
SamplerState s_LinearWrap : register(s3);