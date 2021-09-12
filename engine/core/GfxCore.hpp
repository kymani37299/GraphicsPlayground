#pragma once

#include "GfxCore.h"

namespace GP
{
    template<typename T>
    void GfxDevice::BindConstantBuffer(unsigned int stage, GfxConstantBuffer<T>* constantBuffer, unsigned int binding)
    {
        BindConstantBuffer(stage, constantBuffer->GetBuffer(), binding);
    }

    template<typename T>
    void GfxDevice::BindStructuredBuffer(unsigned int stage, GfxStructuredBuffer<T>* structuredBuffer, unsigned int binding)
    {
        BindStructuredBuffer(stage, structuredBuffer->GetSRV(), binding);
    }

	ENGINE_DLL void ReleaseBuffer(ID3D11Buffer* buffer);
	ENGINE_DLL void ReleaseSRV(ID3D11ShaderResourceView* srv);

	ENGINE_DLL ID3D11Buffer* CreateConstantBuffer(unsigned int bufferSize);
	ENGINE_DLL ID3D11Buffer* CreateStructuredBuffer(unsigned int elementSize, unsigned int numElements);

	ENGINE_DLL ID3D11ShaderResourceView* CreateStructuredBufferView(ID3D11Buffer* structuredBuffer, unsigned int numElements);

	ENGINE_DLL void UploadToBuffer(ID3D11Buffer* buffer, const void* data, unsigned int numBytes, unsigned int offset);

	///////////////////////////////////////////
	/// Constant buffer                  /////
	/////////////////////////////////////////

	template<typename T>
	GfxConstantBuffer<T>::GfxConstantBuffer():
		m_Size(sizeof(T))
	{
		m_Buffer = CreateConstantBuffer(m_Size);
	}

	template<typename T>
	GfxConstantBuffer<T>::~GfxConstantBuffer()
	{
		ReleaseBuffer(m_Buffer);
	}

	template<typename T>
	void GfxConstantBuffer<T>::Upload(const T& data)
	{
		UploadToBuffer(m_Buffer, &data, m_Size, 0);
	}

	///////////////////////////////////////////
	/// Structured buffer                /////
	/////////////////////////////////////////

	template<typename T>
	GfxStructuredBuffer<T>::GfxStructuredBuffer(unsigned int numElements) :
		m_NumElements(numElements),
		m_ElementSize(sizeof(T))
	{
		m_Buffer = CreateStructuredBuffer(m_ElementSize, m_NumElements);
		m_Srv = CreateStructuredBufferView(m_Buffer, m_NumElements);
	}

	template<typename T>
	GfxStructuredBuffer<T>::~GfxStructuredBuffer()
	{
		ReleaseBuffer(m_Buffer);
		ReleaseSRV(m_Srv);
	}

	template<typename T>
	void GfxStructuredBuffer<T>::Upload(const T& data, unsigned int index)
	{
		UploadToBuffer(m_Buffer, &data, m_ElementSize, index * m_ElementSize);
	}
}
