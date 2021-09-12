#pragma once

#include "GfxCore.h"

namespace GP
{
    template<typename T>
    void GfxDevice::BindConstantBuffer(ShaderStage stage, GfxConstantBuffer<T>* constantBuffer, unsigned int binding)
    {
        BindConstantBuffer(stage, constantBuffer->GetBuffer(), binding);
    }

    template<typename T>
    void GfxDevice::BindStructuredBuffer(ShaderStage stage, GfxStructuredBuffer<T>* structuredBuffer, unsigned int binding)
    {
        BindStructuredBuffer(stage, structuredBuffer->GetSRV(), binding);
    }

	void ReleaseBuffer(ID3D11Buffer* buffer);
	void ReleaseSRV(ID3D11ShaderResourceView* srv);

	ID3D11Buffer* CreateConstantBuffer(DxDevice* device, unsigned int bufferSize);
	ID3D11Buffer* CreateStructuredBuffer(DxDevice* device, unsigned int elementSize, unsigned int numElements);

	ID3D11ShaderResourceView* CreateStructuredBufferView(DxDevice* device, ID3D11Buffer* structuredBuffer, unsigned int numElements);

	void UploadToBuffer(DxDevice* device, ID3D11Buffer* buffer, const void* data, unsigned int numBytes, unsigned int offset);

	///////////////////////////////////////////
	/// Constant buffer                  /////
	/////////////////////////////////////////

	template<typename T>
	DxConstantBuffer<T>::DxConstantBuffer(DxDevice* device) :
		m_Device(device),
		m_Size(sizeof(T))
	{
		m_Buffer = CreateConstantBuffer(device, m_Size);
	}

	template<typename T>
	DxConstantBuffer<T>::~DxConstantBuffer()
	{
		ReleaseBuffer(m_Buffer);
	}

	template<typename T>
	void DxConstantBuffer<T>::Upload(const T& data)
	{
		UploadToBuffer(m_Device, m_Buffer, &data, m_Size, 0);
	}

	///////////////////////////////////////////
	/// Structured buffer                /////
	/////////////////////////////////////////

	template<typename T>
	DxStructuredBuffer<T>::DxStructuredBuffer(DxDevice* device, unsigned int numElements) :
		m_Device(device),
		m_NumElements(numElements),
		m_ElementSize(sizeof(T))
	{
		m_Buffer = CreateStructuredBuffer(m_Device, m_ElementSize, m_NumElements);
		m_Srv = CreateStructuredBufferView(m_Device, m_Buffer, m_NumElements);
	}

	template<typename T>
	DxStructuredBuffer<T>::~DxStructuredBuffer()
	{
		ReleaseBuffer(m_Buffer);
		ReleaseSRV(m_Srv);
	}

	template<typename T>
	void DxStructuredBuffer<T>::Upload(const T& data, unsigned int index)
	{
		UploadToBuffer(m_Device, m_Buffer, &data, m_ElementSize, index * m_ElementSize);
	}
}
