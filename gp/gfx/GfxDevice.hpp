#include "GfxDevice.h"

namespace GP
{
	namespace
	{
		inline void ContextOperation(GfxContext* context, const std::string& operationName)
		{
#ifdef CONTEXT_DEBUG
			const std::string contextName = context == g_Device->GetImmediateContext() ? "MAIN" : std::to_string((unsigned int)context->GetHandle());
			const std::string threadName = ThreadUtil::GetThreadName(CURRENT_THREAD);
			LOG("[ExecuteContext][" + contextName + "]" + "[" + threadName + "]" + operationName);
#endif // CONTEXT_DEBUG
		}
	}

	template<typename T>
	inline void GfxContext::BindVertexBuffer(GfxVertexBuffer<T>* vertexBuffer)
	{
		ContextOperation(this, "Bind vertex buffer");
		m_InputAssember.BindVertexBuffer(this, 0, nullptr, 0, 0);
		m_InputAssember.BindVertexBuffer(this, 0, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
	}

	template<typename T>
	inline void GfxContext::BindVertexBufferSlot(GfxVertexBuffer<T>* vertexBuffer, unsigned int slot)
	{
		ContextOperation(this, "Bind vertex buffer slot");
		m_InputAssember.BindVertexBuffer(this, slot, vertexBuffer, vertexBuffer->GetStride(), vertexBuffer->GetOffset());
	}

	inline void GfxContext::BindVertexBufferSlot(std::nullptr_t, unsigned int slot)
	{
		ContextOperation(this, "Bind vertex buffer slot null");
		m_InputAssember.BindVertexBuffer(this, slot, nullptr, 0, 0);
	}
	
	template<typename T> 
	inline void GfxContext::BindInstanceBuffer(GfxInstanceBuffer<T>* instanceBuffer)
	{
		ContextOperation(this, "Bind instance buffer");
		m_InputAssember.BindVertexBuffer(this, 0, nullptr, 0, 0);
		m_InputAssember.BindVertexBuffer(this, 0, instanceBuffer, instanceBuffer->GetStride(), instanceBuffer->GetOffset());
	}

	template<typename T> 
	inline void GfxContext::BindInstanceBufferSlot(GfxInstanceBuffer<T>* instanceBuffer, unsigned int slot)
	{
		ContextOperation(this, "Bind instance buffer slot");
		m_InputAssember.BindVertexBuffer(this, slot, instanceBuffer, instanceBuffer->GetStride(), instanceBuffer->GetOffset());
	}

	inline void GfxContext::BindInstanceBufferSlot(std::nullptr_t, unsigned int slot)
	{
		ContextOperation(this, "Bind instance buffer slot null");
		m_InputAssember.BindVertexBuffer(this, slot, nullptr, 0, 0);
	}

	inline void GfxContext::BindIndexBuffer(GfxIndexBuffer* indexBuffer)
	{
		ContextOperation(this, "Bind index buffer");
		m_InputAssember.BindIndexBuffer(this, indexBuffer);
	}

	inline void GfxContext::BindConstantBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		ContextOperation(this, "Bind contstant buffer");
		BindCB(m_Handle, shaderStage, GetDeviceHandle(this, gfxBuffer), binding);
	}

	inline void GfxContext::BindStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		ContextOperation(this, "Bind structured buffer");
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, gfxBuffer), binding);
	}

	inline void GfxContext::BindRWStructuredBuffer(unsigned int shaderStage, GfxBuffer* gfxBuffer, unsigned int binding)
	{
		ContextOperation(this, "Bind rw structured buffer");
		BindUAV(m_Handle, shaderStage, GetDeviceUAV(this, gfxBuffer), binding);
	}

	inline void GfxContext::BindTexture2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		ContextOperation(this, "Bind texture 2D");
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::BindTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding)
	{
		ContextOperation(this, "Bind texture 3D");
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::BindRWTexture3D(unsigned int shaderStage, GfxBaseTexture3D* texture, unsigned int binding)
	{
		ContextOperation(this, "Bind rw texture 2D");
		BindUAV(m_Handle, shaderStage, GetDeviceUAV(this, texture), binding);
	}

	inline void GfxContext::BindTextureArray2D(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		ContextOperation(this, "Bind texture array 2D");
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::BindCubemap(unsigned int shaderStage, GfxBaseTexture2D* texture, unsigned int binding)
	{
		ContextOperation(this, "Bind cubemap");
		BindSRV(m_Handle, shaderStage, GetDeviceSRV(this, texture), binding);
	}

	inline void GfxContext::UnbindTexture(unsigned int shaderStage, unsigned int binding)
	{
		ContextOperation(this, "Unbind texture");
		BindSRV(m_Handle, shaderStage, nullptr, binding);
	}

	inline void GfxContext::BindSampler(unsigned int shaderStage, GfxSampler* sampler, unsigned int binding)
	{
		ASSERT(binding < g_Device->GetMaxCustomSamplers(), "[GfxDevice::BindSampler] " + std::to_string(binding) + " is out of the limit, maximum binding is " + std::to_string(g_Device->GetMaxCustomSamplers() - 1));
		ContextOperation(this, "Bind sampler");
		BindSamplerState(m_Handle, shaderStage, sampler ? sampler->GetSampler() : nullptr, binding);
	}

	inline void GfxContext::SetDepthStencil(GfxRenderTarget* depthStencil)
	{
		ContextOperation(this, "Set depth stencil");
		m_DepthStencil = depthStencil;
		SetRenderTarget(m_RenderTarget);
	}

	inline void GfxContext::UploadToBuffer(GfxBuffer* gfxBuffer, const void* data, unsigned int numBytes, unsigned int offset)
	{
		ContextOperation(this, "Upload to buffer");
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