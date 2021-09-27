#pragma once

#include "GfxCore.h"
#include "gfx/GfxBuffers.h"

namespace GP
{
    template<typename T>
    void GfxDevice::BindConstantBuffer(unsigned int stage, GfxConstantBuffer<T>* constantBuffer, unsigned int binding)
    {
        BindConstantBuffer(stage, constantBuffer->GetBufferResource(), binding);
    }

    template<typename T>
    void GfxDevice::BindStructuredBuffer(unsigned int stage, GfxStructuredBuffer<T>* structuredBuffer, unsigned int binding)
    {
        BindStructuredBuffer(stage, structuredBuffer->GetBufferResource(), binding);
    }
}
