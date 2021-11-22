#include "GfxBuffers.h"

#include <d3d11_1.h>

#include "gfx/GfxDevice.h"
#include "gfx/GfxResourceHelpers.h"

namespace GP
{
	namespace
	{
		ID3D11Buffer* CreateBuffer(unsigned int byteSize, unsigned int creationFlags, unsigned int structByteStride = 0)
		{
			ID3D11Buffer* buffer;

			D3D11_BUFFER_DESC bufferDesc = GetBufferDesc(byteSize, creationFlags, structByteStride);
			DX_CALL(g_Device->GetDevice()->CreateBuffer(&bufferDesc, NULL, &buffer));

			return buffer;
		}

		ID3D11Buffer* CreateBufferAndUpload(unsigned int byteSize, unsigned int creationFlags, void* data, unsigned int structByteStride = 0)
		{
			ID3D11Buffer* buffer;

			D3D11_BUFFER_DESC bufferDesc = GetBufferDesc(byteSize, creationFlags, structByteStride);
			D3D11_SUBRESOURCE_DATA vertexSubresourceData = { data };
			DX_CALL(g_Device->GetDevice()->CreateBuffer(&bufferDesc, &vertexSubresourceData, &buffer));

			return buffer;
		}
	}

	///////////////////////////////////////////
	/// Buffer resource					 /////
	/////////////////////////////////////////

	GfxBufferResource::~GfxBufferResource()
	{
		ASSERT(m_RefCount == 0, "[~GfxBufferResource] Trying to delete a referenced buffer!");
		SAFE_RELEASE(m_Handle);
	}

	void GfxBufferResource::Initialize()
	{
		if (m_InitializationData)
		{
			m_Handle = CreateBufferAndUpload(m_ByteSize, m_CreationFlags, m_InitializationData, m_Stride);
			free(m_InitializationData);
		}
		else
		{
			m_Handle = CreateBuffer(m_ByteSize, m_CreationFlags, m_Stride);
		}
	}

	void GfxBufferResource::Upload(const void* data, unsigned int numBytes, unsigned int offset)
	{
		ASSERT(Initialized(), "Trying to upload data to an unitialized buffer!");

		ID3D11DeviceContext1* deviceContext = g_Device->GetContext()->GetHandle();

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		DX_CALL(deviceContext->Map(m_Handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
		byte* bufferPtr = (byte*)mappedSubresource.pData;
		memcpy(bufferPtr + offset, data, numBytes);
		deviceContext->Unmap(m_Handle, 0);
	}

	///////////////////////////////////////////
	/// Buffer resource					 /////
	/////////////////////////////////////////

	template<>
	void GfxResource<GfxBufferResource>::Initialize()
	{
		if (!m_Resource->Initialized())
			m_Resource->Initialize();

		unsigned int creationFlags = m_Resource->GetCreationFlags();

		if (creationFlags & RCF_SRV)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = m_Resource->GetNumElements();

			DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetHandle(), &srvDesc, &m_SRV));
		}

		if (creationFlags & RCF_UAV)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = m_Resource->GetNumElements();
			uavDesc.Buffer.Flags = 0;

			DX_CALL(g_Device->GetDevice()->CreateUnorderedAccessView(m_Resource->GetHandle(), &uavDesc, &m_UAV));
		}
	}

	GfxBuffer::~GfxBuffer()
	{
		SAFE_RELEASE(m_Resource);
		SAFE_RELEASE(m_SRV);
		SAFE_RELEASE(m_UAV);
	}
}