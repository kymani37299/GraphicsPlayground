#define CB_CAMERA(x) cbuffer Camera : register(b##x) \
    { \
        float4x4 view; \
        float4x4 projection; \
        float3 cameraPosition; \
    }

CB_CAMERA(0)