#include "GfxBuffers.h"

#include <d3d11_1.h>

#include "gfx/GfxDevice.h"
#include "gfx/GfxResourceHelpers.h"

namespace GP
{
	///////////////////////////////////////////
	/// Buffer resource					 /////
	/////////////////////////////////////////

	GfxBufferResource::~GfxBufferResource()
	{
		ASSERT(m_RefCount == 0, "[~GfxBufferResource] Trying to delete a referenced buffer!");
		SAFE_RELEASE(m_Handle);
	}

	void GfxBufferResource::Initialize(GfxContext* context)
	{
		D3D11_SUBRESOURCE_DATA* subresourceData = nullptr;

		if (m_MemData.numBytes > 0)
		{
			subresourceData = new D3D11_SUBRESOURCE_DATA{ m_MemData.data };
		}

		D3D11_BUFFER_DESC bufferDesc = GetBufferDesc(m_ByteSize, m_CreationFlags, m_Stride);
		DX_CALL(g_Device->GetDevice()->CreateBuffer(&bufferDesc, subresourceData, &m_Handle));

		if (m_MemData.numBytes > 0)
		{
			free(m_MemData.data);
		}
	}

	///////////////////////////////////////////
	/// Buffer resource					 /////
	/////////////////////////////////////////

	template<>
	void GfxResource<GfxBufferResource>::Initialize(GfxContext* context)
	{
		if (!m_Resource->Initialized())
			m_Resource->Initialize(context);

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