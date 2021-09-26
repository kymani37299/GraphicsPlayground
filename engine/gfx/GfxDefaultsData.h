#pragma once

#include "gfx/GfxCommon.h"
#include "gfx/GfxCore.h"
#include "gfx/GfxBuffers.h"

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

	static GfxVertexBuffer::VBData VB_CUBE_DATA()
	{
        GfxVertexBuffer::VBData data = {};
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

    static GfxVertexBuffer::VBData VB_2DQUAD_DATA()
    {
        GfxVertexBuffer::VBData data = {};
        data.pData = (void*)&QUAD2D_VERTICES;
        data.numBytes = sizeof(QUAD2D_VERTICES);
        data.stride = 4 * sizeof(float);
        return data;
    }

    float QUAD_VERTICES[] = { // (x, y, z), (u, v)
    -1.0f, 0.0f,  1.0f, 0.f, 0.f,
    1.0f, 0.0f, -1.0f, 1.f, 1.f,
    -1.0f, 0.0f, -1.0f, 0.f, 1.f,
    -1.0f, 0.0f,  1.0f, 0.f, 0.f,
    1.0f, 0.0f,  1.0f, 1.f, 0.f,
    1.0f, 0.0f, -1.0f, 1.f, 1.f
    };

    static GfxVertexBuffer::VBData VB_QUAD_DATA()
    {
        GfxVertexBuffer::VBData data = {};
        data.pData = (void*)&QUAD_VERTICES;
        data.numBytes = sizeof(QUAD_VERTICES);
        data.stride = 5 * sizeof(float);
        return data;
    }
}

