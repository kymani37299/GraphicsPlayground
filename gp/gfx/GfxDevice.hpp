#include "GfxDevice.h"

namespace GP
{
	template<typename T>
	inline void GfxContext::BindVertexBuffer(GfxVertexBuffer<T>* vertexBuffer)
	{
		m_InputAssember.BindVertexBuffer(0, nullptr, 0, 0);
		m_InputAssember.BindVertexBuffer(0, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
	}

	template<typename T>
	inline void GfxContext::BindVertexBufferSlot(GfxVertexBuffer<T>* vertexBuffer, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(slot, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
	}

	inline void GfxContext::BindVertexBufferSlot(std::nullptr_t, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(slot, nullptr, 0, 0);
	}
	
	template<typename T> 
	inline void GfxContext::BindInstanceBuffer(GfxInstanceBuffer<T>* instanceBuffer)
	{
		m_InputAssember.BindVertexBuffer(0, nullptr, 0, 0);
		m_InputAssember.BindVertexBuffer(0, instanceBuffer, instanceBuffer->GetStride(), instanceBuffer->GetOffset());
	}

	template<typename T> 
	inline void GfxContext::BindInstanceBufferSlot(GfxInstanceBuffer<T>* instanceBuffer, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(slot, instanceBuffer, instanceBuffer->GetStride(), instanceBuffer->GetOffset());
	}

	inline void GfxContext::BindInstanceBufferSlot(std::nullptr_t, unsigned int slot)
	{
		m_InputAssember.BindVertexBuffer(slot, nullptr, 0, 0);
	}

	inline void GfxContext::BindIndexBuffer(GfxIndexBuffer* indexBuffer)
	{
		m_InputAssember.BindIndexBuffer(indexBuffer);
	}

	inline void GfxContext::SetPrimitiveTopology(PrimitiveTopology primitiveTopology)
	{
		m_InputAssember.SetPrimitiveTopology(primitiveTopology);
	}

	inline void GfxContext::BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		BindCB(m_Handles[m_Current], shaderStage, GetDeviceHandle(gfxBuffer), binding);
	}

	inline void GfxContext::BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(gfxBuffer), binding);
	}

	inline void GfxContext::BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		BindUAV(m_Handles[m_Current], shaderStage, GetDeviceUAV(gfxBuffer), binding);
	}

	inline void GfxContext::BindTexture2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(texture), binding);
	}

	inline void GfxContext::BindTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding)
	{
		BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(texture), binding);
	}

	inline void GfxContext::BindRWTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding)
	{
		BindUAV(m_Handles[m_Current], shaderStage, GetDeviceUAV(texture), binding);
	}

	inline void GfxContext::BindTextureArray2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(texture), binding);
	}

	inline void GfxContext::BindCubemap(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		BindSRV(m_Handles[m_Current], shaderStage, GetDeviceSRV(texture), binding);
	}

	inline void GfxContext::UnbindTexture(unsigned int shaderStage, unsigned int binding)
	{
		BindSRV(m_Handles[m_Current], shaderStage, nullptr, binding);
	}

	inline void GfxContext::BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding)
	{
		ASSERT(binding < g_Device->GetMaxCustomSamplers(), "[GfxDevice::BindSampler] " + std::to_string(binding) + " is out of the limit, maximum binding is " + std::to_string(g_Device->GetMaxCustomSamplers() - 1));
		BindSamplerState(m_Handles[m_Current], shaderStage, sampler ? sampler->GetSampler() : nullptr, binding);
	}
}