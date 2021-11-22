#pragma once

#include "gfx/GfxCommon.h"
#include "gfx/GfxResource.h"

struct ID3D11Buffer;

namespace GP
{
	class GfxBufferResource : public GfxResourceHandle<ID3D11Buffer>
	{
		DELETE_COPY_CONSTRUCTOR(GfxBufferResource);
	public:
		GfxBufferResource(unsigned int byteSize, unsigned int stride, unsigned int creationFlags = 0):
			GfxResourceHandle(creationFlags),
			m_ByteSize(byteSize),
			m_Stride(stride) 
		{
			ASSERT(m_Stride, "[GfxBufferResource] Buffer stride cannot be zero!");
		}

		GP_DLL void Initialize();
		GP_DLL void Upload(const void* data, unsigned int numBytes, unsigned int offset = 0);

		inline unsigned int GetByteSize() const { return m_ByteSize; }
		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetNumElements() const { return m_ByteSize / m_Stride; }

		inline void SetInitializationData(void* data)
		{
			ASSERT(!Initialized(), "Trying to add a initialization data to already initialized buffer!");
			m_InitializationData = malloc(m_ByteSize);
			memcpy(m_InitializationData, data, m_ByteSize);
		}

	private:
		~GfxBufferResource();

	private:
		void* m_InitializationData = nullptr;
		unsigned int m_ByteSize;
		unsigned int m_Stride;
	};

	class GfxBuffer : public GfxResource<GfxBufferResource>
	{
	protected:
		using GfxResource::GfxResource;
	public:
		GP_DLL ~GfxBuffer();

		void Upload(void* data, unsigned int numBytes, unsigned int offset = 0) 
		{
			if (!Initialized())
			{
				m_Resource->AddCreationFlags(RCF_CPUWrite);
				Initialize();
			}
			m_Resource->Upload(data, numBytes, offset);
		}
	};

	template<typename T>
	class GfxVertexBuffer : public GfxBuffer
	{
	public:
		GfxVertexBuffer<T>(unsigned int numVertices):
			GfxBuffer(ResourceType::VertexBufer),
			m_NumVerts(numVertices),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numVertices * sizeof(T), sizeof(T), RCF_VB);
		}

		GfxVertexBuffer<T>(void* data, unsigned int numVertices):
			GfxBuffer(ResourceType::VertexBufer),
			m_NumVerts(numVertices),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numVertices * sizeof(T), sizeof(T), RCF_VB);
			m_Resource->SetInitializationData(data);
		}

		GfxVertexBuffer<T>(GfxBuffer* buffer) :
			GfxBuffer(ResourceType::VertexBufer, buffer->GetResource()),
			m_NumVerts(m_BufferResource->GetByteSize() / sizeof(T)),
			m_Offset(0) 
		{
			m_Resource->AddCreationFlags(RCF_VB);
		}

		inline unsigned int GetStride() const { return sizeof(T); }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumVerts() const { return m_NumVerts; }

		// Do this but with [] overload
		//template<typename T>
		//inline void Upload(const T& value, unsigned int index)
		//{
		//	GfxBuffer::Upload((void*) &value, GetStride(), GetOffset() + GetStride() * index);
		//}

	private:
		unsigned int m_Offset = 0;
		unsigned int m_NumVerts = 0;
	};

	
	template<typename T>
	using GfxInstanceBuffer = GfxVertexBuffer<T>;

	class GfxIndexBuffer : public GfxBuffer
	{
	public:
		GfxIndexBuffer(unsigned int numIndices, unsigned int stride = sizeof(unsigned int)):
			GfxBuffer(ResourceType::IndexBuffer),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numIndices * stride, stride, RCF_IB);
		}

		GfxIndexBuffer(void* pIndices, unsigned int numIndices, unsigned int stride = sizeof(unsigned int)):
			GfxBuffer(ResourceType::IndexBuffer),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0) 
		{
			m_Resource = new GfxBufferResource(numIndices * stride, stride, RCF_IB);
			m_Resource->SetInitializationData(pIndices);
		}

		GfxIndexBuffer(GfxBuffer* buffer, unsigned int numIndices, unsigned int stride = sizeof(unsigned int)) :
			GfxBuffer(ResourceType::IndexBuffer, buffer->GetResource()),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0) 
		{
			m_Resource->AddCreationFlags(RCF_IB);
		}

		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumIndices() const { return m_NumIndices; }

	private:
		unsigned int m_Stride = 0;
		unsigned int m_Offset = 0;
		unsigned int m_NumIndices = 0;
	};

	template<typename T>
	class GfxConstantBuffer : public GfxBuffer
	{
	public:
		GfxConstantBuffer<T>():
			GfxBuffer(ResourceType::ConstantBuffer)
		{
			m_Resource = new GfxBufferResource(sizeof(T) + 0xf & 0xfffffff0, sizeof(T), RCF_CB | RCF_CPUWrite);
		}

		GfxConstantBuffer<T>(GfxBuffer* buffer) :
			GfxBuffer(ResourceType::ConstantBuffer, buffer->GetResource())
		{
			m_Resource->AddCreationFlags(BCF_ConstantBuffer);
		}

		inline void Upload(const T& data) { GfxBuffer::Upload((void*) &data, sizeof(T)); }
	};

	template<typename T>
	class GfxStructuredBuffer : public GfxBuffer
	{
	public:
		GfxStructuredBuffer<T>(unsigned int numElements):
			GfxBuffer(ResourceType::StructuredBuffer),
			m_NumElements(numElements) 
		{
			m_Resource = new GfxBufferResource(sizeof(T) * numElements, sizeof(T), RCF_SRV | RCF_StructuredBuffer);
		}

		GfxStructuredBuffer<T>(GfxBuffer* buffer, unsigned int numElements):
			GfxBuffer(ResourceType::StructuredBuffer, buffer->GetResource()),
			m_NumElements(numElements) 
		{
			m_Resource->AddCreationFlags(RCF_SRV | RCF_StructuredBuffer);
		}

		inline void Upload(const T& data, unsigned int index)
		{
			ASSERT(index < m_NumElements, "Structured buffer overflow!");
			GfxBuffer::Upload(&data, sizeof(T), index * sizeof(T));
		}

	private:
		unsigned int m_NumElements;
	};
}