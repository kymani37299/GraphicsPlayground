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

	ID3D11Buffer* CreateConstantBuffer(unsigned int bufferSize);
	ID3D11Buffer* CreateStructuredBuffer(unsigned int elementSize, unsigned int numElements);

	ID3D11ShaderResourceView* CreateStructuredBufferView(ID3D11Buffer* structuredBuffer, unsigned int numElements);

	void UploadToBuffer(D3D11Buffer* buffer, const void* data, unsigned int numBytes, unsigned int offset);

	///////////////////////////////////////////
	/// Constant buffer                  /////
	/////////////////////////////////////////

	template<typename T>
	DxConstantBuffer<T>::DxConstantBuffer() :
		m_Size(sizeof(T))
	{
		m_Buffer = CreateConstantBuffer(m_Size);
	}

	template<typename T>
	DxConstantBuffer<T>::~DxConstantBuffer()
	{
		ReleaseBuffer(m_Buffer);
	}

	template<typename T>
	void DxConstantBuffer<T>::Upload(const T& data)
	{
		UploadToBuffer(m_Buffer, &data, m_Size, 0);
	}

	///////////////////////////////////////////
	/// Structured buffer                /////
	/////////////////////////////////////////

	template<typename T>
	DxStructuredBuffer<T>::DxStructuredBuffer(unsigned int numElements) :
		m_NumElements(numElements),
		m_ElementSize(sizeof(T))
	{
		m_Buffer = CreateStructuredBuffer(m_ElementSize, m_NumElements);
		m_Srv = CreateStructuredBufferView(m_Buffer, m_NumElements);
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
		UploadToBuffer(m_Buffer, &data, m_ElementSize, index * m_ElementSize);
	}
}
