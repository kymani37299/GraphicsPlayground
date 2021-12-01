#include "GfxDevice.h"

namespace GP
{
	template<typename T>
	inline void GfxContext::BindVertexBuffer(GfxVertexBuffer<T>* vertexBuffer)
	{
		m_InputAssember.BindVertexBuffer(this, 0, nullptr, 0, 0);
		m_InputAssember.BindVertexBuffer(this, 0, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
	}

	template<typename T>
	inline void GfxContext::BindVertexBufferSlot(GfxVertexBuffer<T>* vertexBuffer, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(this, slot, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
	}

	inline void GfxContext::BindVertexBufferSlot(std::nullptr_t, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(this, slot, nullptr, 0, 0);
	}
	
	template<typename T> 
	inline void GfxContext::BindInstanceBuffer(GfxInstanceBuffer<T>* instanceBuffer)
	{
		m_InputAssember.BindVertexBuffer(this, 0, nullptr, 0, 0);
		m_InputAssember.BindVertexBuffer(this, 0, instanceBuffer, instanceBuffer->GetStride(), instanceBuffer->GetOffset());
	}

	template<typename T> 
	inline void GfxContext::BindInstanceBufferSlot(GfxInstanceBuffer<T>* instanceBuffer, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(this, slot, instanceBuffer, instanceBuffer->GetStride(), instanceBuffer->GetOffset());
	}

	inline void GfxContext::BindInstanceBufferSlot(std::nullptr_t, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(this, slot, nullptr, 0, 0);
	}

	inline void GfxContext::BindIndexBuffer(GfxIndexBuffer* indexBuffer)
	{
		m_InputAssember.BindIndexBuffer(this, indexBuffer);
	}

	inline void GfxContext::BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		BindCB(m_Handle, shaderStage, GetDeviceHandle(this, gfxBuffer), binding);
	}

	inline void GfxContext::BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, gfxBuffer), binding);
	}

	inline void GfxContext::BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		BindUAV(m_Handle, shaderStage, GetDeviceUAV(this, gfxBuffer), binding);
	}

	inline void GfxContext::BindTexture2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::BindTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding)
	{
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::BindRWTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding)
	{
		BindUAV(m_Handle, shaderStage, GetDeviceUAV(this, texture), binding);
	}

	inline void GfxContext::BindTextureArray2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::BindCubemap(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::UnbindTexture(unsigned int shaderStage, unsigned int binding)
	{
		BindSRV(m_Handle, shaderStage, nullptr, binding);
	}

	inline void GfxContext::BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding)
	{
		ASSERT(binding < g_Device->GetMaxCustomSamplers(), "[GfxDevice::BindSampler] " + std::to_string(binding) + " is out of the limit, maximum binding is " + std::to_string(g_Device->GetMaxCustomSamplers() - 1));
		BindSamplerState(m_Handle, shaderStage, sampler ? sampler->GetSampler() : nullptr, binding);
	}

	inline void GfxContext::SetDepthStencil(GfxRenderTarget* depthStencil)
	{
		m_DepthStencil = depthStencil;
		SetRenderTarget(m_RenderTarget);
	}

	inline void GfxContext::UploadToBuffer(GfxBuffer* gfxBuffer, const void* data, unsigned int numBytes, unsigned int offset)
	{
		unsigned char* bufferData = (unsigned char*)Map(gfxBuffer, false, true);
		memcpy(bufferData + offset, data, numBytes);
		Unmap(gfxBuffer);
	}

	template<typename T>
	inline void GfxContext::UploadToBuffer(GfxConstantBuffer<T>* constantBuffer, const T& data)
	{
		UploadToBuffer(constantBuffer, (const void*) &data, sizeof(T), 0);
	}

	template<typename T>
	inline void GfxContext::UploadToBuffer(GfxStructuredBuffer<T>* structuredBuffer, const T& data, unsigned int index)
	{
		UploadToBuffer(structuredBuffer, (const void*) &data, sizeof(T), sizeof(T) * index);
	}

	inline void GfxContext::UploadToTexture(GfxBaseTexture2D* texture, void* data, unsigned int arrayIndex)
	{
		if (!texture->Initialized())
		{
			texture->AddCreationFlags(RCF_CopyDest);
			texture->Initialize(this);
		}
		
		UploadToTexture(texture->GetResource(), data, arrayIndex);
	}
}