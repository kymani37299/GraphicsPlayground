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

		GP_DLL void Initialize(GfxContext* context);

		inline unsigned int GetByteSize() const { return m_ByteSize; }
		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetNumElements() const { return m_ByteSize / m_Stride; }

	private:
		~GfxBufferResource();

	private:
		unsigned int m_ByteSize;
		unsigned int m_Stride;
	};

	class GfxBuffer : public GfxResource<GfxBufferResource>
	{
	protected:
		using GfxResource::GfxResource;
	public:
		GP_DLL ~GfxBuffer();
	};

	template<typename T>
	class GfxVertexBuffer : public GfxBuffer
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_VB;
	public:
		GfxVertexBuffer<T>(unsigned int numVertices):
			GfxBuffer(ResourceType::VertexBufer),
			m_NumVerts(numVertices),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numVertices * sizeof(T), sizeof(T), DEFAULT_FLAGS);
		}

		GfxVertexBuffer<T>(void* data, unsigned int numVertices):
			GfxBuffer(ResourceType::VertexBufer),
			m_NumVerts(numVertices),
			m_Offset(0)
		{
			unsigned int byteSize = numVertices * sizeof(T);
			m_Resource = new GfxBufferResource(byteSize, sizeof(T), DEFAULT_FLAGS);
			m_Resource->SetInitializationData(data, byteSize);
		}

		GfxVertexBuffer<T>(GfxBuffer* buffer) :
			GfxBuffer(ResourceType::VertexBufer, buffer->GetResource()),
			m_NumVerts(m_BufferResource->GetByteSize() / sizeof(T)),
			m_Offset(0) 
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

		inline unsigned int GetStride() const { return sizeof(T); }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumVerts() const { return m_NumVerts; }

	private:
		unsigned int m_Offset = 0;
		unsigned int m_NumVerts = 0;
	};

	
	template<typename T>
	using GfxInstanceBuffer = GfxVertexBuffer<T>;

	class GfxIndexBuffer : public GfxBuffer
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_IB;
	public:
		GfxIndexBuffer(unsigned int numIndices, unsigned int stride = sizeof(unsigned int)):
			GfxBuffer(ResourceType::IndexBuffer),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numIndices * stride, stride, DEFAULT_FLAGS);
		}

		GfxIndexBuffer(void* pIndices, unsigned int numIndices, unsigned int stride = sizeof(unsigned int)):
			GfxBuffer(ResourceType::IndexBuffer),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0) 
		{
			const unsigned int byteSize = numIndices * stride;
			m_Resource = new GfxBufferResource(byteSize, stride, DEFAULT_FLAGS);
			m_Resource->SetInitializationData(pIndices, byteSize);
		}

		GfxIndexBuffer(GfxBuffer* buffer, unsigned int numIndices, unsigned int stride = sizeof(unsigned int)) :
			GfxBuffer(ResourceType::IndexBuffer, buffer->GetResource()),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0) 
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
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
		static constexpr unsigned int DEFAULT_FLAGS = RCF_CB | RCF_CPUWrite;
	public:
		GfxConstantBuffer<T>():
			GfxBuffer(ResourceType::ConstantBuffer)
		{
			m_Resource = new GfxBufferResource(sizeof(T) + 0xf & 0xfffffff0, sizeof(T), DEFAULT_FLAGS);
		}

		GfxConstantBuffer<T>(GfxBuffer* buffer) :
			GfxBuffer(ResourceType::ConstantBuffer, buffer->GetResource())
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}
	};

	template<typename T>
	class GfxStructuredBuffer : public GfxBuffer
	{
		static constexpr unsigned int DEFAULT_FLAGS = RCF_SRV | RCF_StructuredBuffer;
	public:
		GfxStructuredBuffer<T>(unsigned int numElements):
			GfxBuffer(ResourceType::StructuredBuffer),
			m_NumElements(numElements) 
		{
			m_Resource = new GfxBufferResource(sizeof(T) * numElements, sizeof(T), DEFAULT_FLAGS);
		}

		GfxStructuredBuffer<T>(GfxBuffer* buffer, unsigned int numElements):
			GfxBuffer(ResourceType::StructuredBuffer, buffer->GetResource()),
			m_NumElements(numElements) 
		{
			m_Resource->AddCreationFlags(DEFAULT_FLAGS);
		}

	private:
		unsigned int m_NumElements;
	};
}