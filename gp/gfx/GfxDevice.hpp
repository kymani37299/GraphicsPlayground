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
}