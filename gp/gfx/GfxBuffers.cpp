#include "GfxBuffers.h"

#include <d3d11_1.h>

#include "gfx/GfxDevice.h"

namespace GP
{
	namespace
	{
		inline D3D11_USAGE GetBufferUsage(unsigned int creationFlags)
		{
			if (creationFlags & BCF_Usage_Immutable)	return D3D11_USAGE_IMMUTABLE;
			else if (creationFlags & BCF_Usage_Dynamic) return D3D11_USAGE_DYNAMIC;
			else if (creationFlags & BCF_Usage_Staging) return D3D11_USAGE_STAGING;
			return D3D11_USAGE_DEFAULT;
		}

		inline unsigned int GetBufferBindFlags(unsigned int creationFlags)
		{
			unsigned int flags = 0;
			if (creationFlags & BCF_VertexBuffer)		flags |= D3D11_BIND_VERTEX_BUFFER;
			if (creationFlags & BCF_IndexBuffer)		flags |= D3D11_BIND_INDEX_BUFFER;
			if (creationFlags & BCF_ConstantBuffer)		flags |= D3D11_BIND_CONSTANT_BUFFER;
			if (creationFlags & BCF_SRV)				flags |= D3D11_BIND_SHADER_RESOURCE;
			if (creationFlags & BCF_UAV)				flags |= D3D11_BIND_UNORDERED_ACCESS;
			return flags;
		}

		inline unsigned int GetBufferAccessFlags(unsigned int creationFlags)
		{
			unsigned int flags = 0;
			if (creationFlags & BCF_CPURead)	flags |= D3D11_CPU_ACCESS_READ;
			if (creationFlags & BCF_CPUWrite)	flags |= D3D11_CPU_ACCESS_WRITE;
			return flags;
		}

		inline unsigned int GetBufferMiscFlags(unsigned int creationFlags)
		{
			unsigned int flags = 0;
			if (creationFlags & BCF_StructuredBuffer) flags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			return flags;
		}

		D3D11_BUFFER_DESC GetBufferDesc(unsigned int byteSize, unsigned int creationFlags, unsigned int structByteStride)
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.ByteWidth = byteSize;
			bufferDesc.Usage = GetBufferUsage(creationFlags);
			bufferDesc.BindFlags = GetBufferBindFlags(creationFlags);
			bufferDesc.CPUAccessFlags = GetBufferAccessFlags(creationFlags);
			bufferDesc.MiscFlags = GetBufferMiscFlags(creationFlags);
			bufferDesc.StructureByteStride = creationFlags & BCF_StructuredBuffer ? structByteStride : 0;
			return bufferDesc;
		}

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
		SAFE_RELEASE(m_Buffer);
	}

	void GfxBufferResource::Initialize()
	{
		if (m_InitializationData)
		{
			m_Buffer = CreateBufferAndUpload(m_ByteSize, m_CreationFlags, m_InitializationData, m_Stride);
			free(m_InitializationData);
		}
		else
		{
			m_Buffer = CreateBuffer(m_ByteSize, m_CreationFlags, m_Stride);
		}
	}

	void GfxBufferResource::Upload(const void* data, unsigned int numBytes, unsigned int offset)
	{
		ASSERT(m_Buffer != nullptr, "Trying to upload data to an unitialized buffer!");

		ID3D11DeviceContext1* deviceContext = g_Device->GetContext()->GetHandle();

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		DX_CALL(deviceContext->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));
		byte* bufferPtr = (byte*)mappedSubresource.pData;
		memcpy(bufferPtr + offset, data, numBytes);
		deviceContext->Unmap(m_Buffer, 0);
	}

	///////////////////////////////////////////
	/// Buffer resource					 /////
	/////////////////////////////////////////

	void GfxBuffer::Initialize()
	{
		if (!m_Resource->Initialized())
			m_Resource->Initialize();

		unsigned int creationFlags = m_Resource->GetCreationFlags();

		if (creationFlags & BCF_SRV)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			srvDesc.Buffer.FirstElement = 0;
			srvDesc.Buffer.NumElements = m_Resource->GetNumElements();

			DX_CALL(g_Device->GetDevice()->CreateShaderResourceView(m_Resource->GetBuffer(), &srvDesc, &m_SRV));
		}

		if (creationFlags & BCF_UAV)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.FirstElement = 0;
			uavDesc.Buffer.NumElements = m_Resource->GetNumElements();
			uavDesc.Buffer.Flags = 0;

			DX_CALL(g_Device->GetDevice()->CreateUnorderedAccessView(m_Resource->GetBuffer(), &uavDesc, &m_UAV));
		}
	}

	GfxBuffer::~GfxBuffer()
	{
		SAFE_RELEASE(m_Resource);
		SAFE_RELEASE(m_SRV);
		SAFE_RELEASE(m_UAV);
	}
}