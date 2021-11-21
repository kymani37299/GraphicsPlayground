#pragma once

#include "gfx/GfxCommon.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace GP
{
	enum BufferCreationFlags
	{
		BCF_VertexBuffer = 1 << 0,
		BCF_IndexBuffer = 1 << 1,
		BCF_ConstantBuffer = 1 << 2,
		BCF_StructuredBuffer = 1 << 3,
		BCF_SRV = 1 << 4,
		BCF_UAV = 1 << 5,

		BCF_CPURead = 1 << 6,
		BCF_CPUWrite = 1 << 7,
		BCF_CPUReadWrite = BCF_CPURead | BCF_CPUWrite,
		BCF_CopyDest = 1 << 8
	};

	class GfxBufferResource
	{
		DELETE_COPY_CONSTRUCTOR(GfxBufferResource);
	public:
		GfxBufferResource(unsigned int byteSize, unsigned int stride, unsigned int creationFlags = 0):
			m_ByteSize(byteSize),
			m_CreationFlags(creationFlags),
			m_Stride(stride) 
		{
			ASSERT(m_Stride, "[GfxBufferResource] Buffer stride cannot be zero!");
		}

		GP_DLL void Initialize();
		GP_DLL void Upload(const void* data, unsigned int numBytes, unsigned int offset = 0);

		inline void AddCreationFlags(unsigned int creationFlags)
		{
			if (!m_Buffer) LOG("[Warning][GfxBufferResource] Trying to add creation flags to already initialized resource!");
			m_CreationFlags |= creationFlags;
		}

		inline void AddRef()
		{
			m_RefCount++;
		}

		inline void Release()
		{
			m_RefCount--;
			if (m_RefCount == 0) delete this;
		}

		inline bool Initialized() const { return m_Buffer != nullptr; }
		inline unsigned int GetByteSize() const { return m_ByteSize; }
		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetNumElements() const { return m_ByteSize / m_Stride; }
		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
		inline unsigned int GetCreationFlags() const { return m_CreationFlags; }

		inline void SetInitializationData(void* data)
		{
			ASSERT(m_Buffer == nullptr, "Trying to add a initialization data to already initialized buffer!");
			m_InitializationData = malloc(m_ByteSize);
			memcpy(m_InitializationData, data, m_ByteSize);
		}

	private:
		~GfxBufferResource();

	private:
		void* m_InitializationData = nullptr;

		ID3D11Buffer* m_Buffer = nullptr;

		unsigned int m_ByteSize;
		unsigned int m_Stride;
		unsigned int m_CreationFlags;
		unsigned int m_RefCount = 1;
	};

	class GfxBuffer
	{
		DELETE_COPY_CONSTRUCTOR(GfxBuffer);
	protected:
		GfxBuffer() {}

		GfxBuffer(GfxBufferResource* bufferResource):
			m_Resource(bufferResource)
		{
			m_Resource->AddRef();
		}

	public:
		GP_DLL ~GfxBuffer();

		inline bool Initialized() const 
		{
			const unsigned int creationFlags = m_Resource->GetCreationFlags();
			bool srvOK = m_SRV || !(creationFlags & BCF_SRV);
			bool uavOK = m_UAV || !(creationFlags & BCF_UAV);
			return m_Resource->Initialized() && srvOK && uavOK;
		}

		GP_DLL void Initialize();
		inline void SetInitializationData(void* data) { m_Resource->SetInitializationData(data); }

		inline GfxBufferResource* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
		inline ID3D11UnorderedAccessView* GetUAV() const { return m_UAV; }

		void Upload(void* data, unsigned int numBytes, unsigned int offset = 0) 
		{
			if (!Initialized())
			{
				m_Resource->AddCreationFlags(BCF_CPUWrite);
				Initialize();
			}
			m_Resource->Upload(data, numBytes, offset);
		}

		inline void AddCreationFlags(unsigned int flags) { m_Resource->AddCreationFlags(flags); }

	protected:
		GfxBufferResource* m_Resource = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView* m_UAV = nullptr;
	};

	template<typename T>
	class GfxVertexBuffer : public GfxBuffer
	{
	public:
		GfxVertexBuffer<T>(unsigned int numVertices):
			m_NumVerts(numVertices),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numVertices * sizeof(T), sizeof(T), BCF_VertexBuffer);
		}

		GfxVertexBuffer<T>(void* data, unsigned int numVertices):
			m_NumVerts(numVertices),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numVertices * sizeof(T), sizeof(T), BCF_VertexBuffer);
			m_Resource->SetInitializationData(data);
		}

		GfxVertexBuffer<T>(GfxBuffer* buffer) :
			GfxBuffer(buffer->GetResource()),
			m_NumVerts(m_BufferResource->GetByteSize() / sizeof(T)),
			m_Offset(0) 
		{
			m_Resource->AddCreationFlags(BCF_VertexBuffer);
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
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0)
		{
			m_Resource = new GfxBufferResource(numIndices * stride, stride, BCF_IndexBuffer);
		}

		GfxIndexBuffer(void* pIndices, unsigned int numIndices, unsigned int stride = sizeof(unsigned int)):
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0) 
		{
			m_Resource = new GfxBufferResource(numIndices * stride, stride, BCF_IndexBuffer);
			m_Resource->SetInitializationData(pIndices);
		}

		GfxIndexBuffer(GfxBuffer* buffer, unsigned int numIndices, unsigned int stride = sizeof(unsigned int)) :
			GfxBuffer(buffer->GetResource()),
			m_NumIndices(numIndices),
			m_Stride(stride),
			m_Offset(0) 
		{
			m_Resource->AddCreationFlags(BCF_IndexBuffer);
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
		GfxConstantBuffer<T>()
		{
			m_Resource = new GfxBufferResource(sizeof(T) + 0xf & 0xfffffff0, sizeof(T), BCF_ConstantBuffer | BCF_CPUWrite);
		}

		GfxConstantBuffer<T>(GfxBuffer* buffer) :
			GfxBuffer(buffer->GetResource()) 
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
			m_NumElements(numElements) 
		{
			m_Resource = new GfxBufferResource(sizeof(T) * numElements, sizeof(T), BCF_SRV | BCF_StructuredBuffer);
		}

		GfxStructuredBuffer<T>(GfxBuffer* buffer, unsigned int numElements) :
			GfxBuffer(buffer->GetResource()),
			m_NumElements(numElements) 
		{
			m_Resource->AddCreationFlags(BCF_SRV | BCF_StructuredBuffer);
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