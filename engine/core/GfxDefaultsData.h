#pragma once

#include "GfxCore.h"

namespace GP::Data
{
    static const float CUBE_VERTICES[] = { // (x,y,z)
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

	static VertexBufferData VB_CUBE_DATA()
	{
        VertexBufferData data = {};
        data.pData = (void*) &CUBE_VERTICES;
        data.numBytes = sizeof(CUBE_VERTICES);
        data.stride = 3 * sizeof(float);
        return data;
	}

    float QUAD2D_VERTICES[] = { // (x, y), (u, v)
        -1.0f,  1.0f, 0.f, 0.f,
        1.0f, -1.0f, 1.f, 1.f,
        -1.0f, -1.0f, 0.f, 1.f,
        -1.0f,  1.0f, 0.f, 0.f,
        1.0f,  1.0f, 1.f, 0.f,
        1.0f, -1.0f, 1.f, 1.f
    };

    static VertexBufferData VB_2DQUAD_DATA()
    {
        VertexBufferData data = {};
        data.pData = (void*)&QUAD2D_VERTICES;
        data.numBytes = sizeof(QUAD2D_VERTICES);
        data.stride = 4 * sizeof(float);
        return data;
    }
}

