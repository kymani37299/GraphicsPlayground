#pragma once

namespace GP::Data
{
    struct VBPos
    {
        float x;
        float y;
        float z;
    };

    struct VBPos2D
    {
        float x;
        float y;
    };

    struct VBTex
    {
        float u;
        float v;
    };

    struct VBPosTex
    {
        VBPos pos;
        VBTex tex;
    };

    struct VBPosTex2D
    {
        VBPos2D pos;
        VBTex tex;
    };

    typedef VBPos VB_CUBE_TYPE;
    static const VBPos VB_CUBE_DATA[] = { // (x,y,z)
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f, 
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, 
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
    };
    const unsigned int VB_CUBE_SIZE = sizeof(VB_CUBE_DATA) / sizeof(VB_CUBE_TYPE);

    typedef VBPosTex2D VB_QUAD2D_TYPE;
    static float VB_QUAD2D_DATA[] = { // (x, y), (u, v)
        -1.0f, -1.0f, 0.f, 1.f,
        1.0f, -1.0f, 1.f, 1.f,
        -1.0f,  1.0f, 0.f, 0.f,
        1.0f, -1.0f, 1.f, 1.f,
        1.0f,  1.0f, 1.f, 0.f,
        -1.0f,  1.0f, 0.f, 0.f,
    };
    const unsigned int VB_QUAD2D_SIZE = sizeof(VB_QUAD2D_DATA) / sizeof(VB_QUAD2D_TYPE);

    typedef VBPosTex VB_QUAD_TYPE;
    static float VB_QUAD_DATA[] = { // (x, y, z), (u, v)
    -1.0f, 0.0f,  1.0f, 0.f, 0.f,
    1.0f, 0.0f, -1.0f, 1.f, 1.f,
    -1.0f, 0.0f, -1.0f, 0.f, 1.f,
    -1.0f, 0.0f,  1.0f, 0.f, 0.f,
    1.0f, 0.0f,  1.0f, 1.f, 0.f,
    1.0f, 0.0f, -1.0f, 1.f, 1.f
    };
    const unsigned int VB_QUAD_SIZE = sizeof(VB_QUAD_DATA) / sizeof(VB_QUAD_TYPE);
}

