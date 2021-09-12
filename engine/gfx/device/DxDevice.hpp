#include "DxDevice.h"

#include "gfx/device/DxBuffers.h"

template<typename T>
void DxDevice::BindConstantBuffer(ShaderStage stage, DxConstantBuffer<T>* constantBuffer, unsigned int binding)
{
    BindConstantBuffer(stage, constantBuffer->GetBuffer() , binding);
}

template<typename T> 
void DxDevice::BindStructuredBuffer(ShaderStage stage, DxStructuredBuffer<T>* structuredBuffer, unsigned int binding)
{
    BindStructuredBuffer(stage, structuredBuffer->GetSRV() , binding);
}