#pragma once

#include "GfxCommon.h"

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

		BCF_Usage_Immutable = 1 << 6,
		BCF_Usage_Dynamic = 1 << 7,
		BCF_Usage_Staging = 1 << 8,

		BCF_CPURead = 1 << 9,
		BCF_CPUWrite = 1 << 10,
		BCF_CPUReadWrite = BCF_CPURead | BCF_CPUWrite
	};

	class GfxBufferResource
	{
		DELETE_COPY_CONSTRUCTOR(GfxBufferResource);
	public:
		ENGINE_DLL GfxBufferResource(unsigned int byteSize, unsigned int creationFlags = 0, unsigned int elementStride = 0):
			m_ByteSize(byteSize),
			m_CreationFlags(creationFlags),
			m_ElementStride(elementStride) {}

		ENGINE_DLL void Initialize();
		ENGINE_DLL void Upload(const void* data, unsigned int numBytes, unsigned int offset = 0);

		inline void AddCreationFlags(unsigned int creationFlags)
		{
			ASSERT(m_Buffer == nullptr, "Trying to add a creation flags to already initialized buffer!");
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
		inline ID3D11Buffer* GetBuffer() const { return m_Buffer; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
		inline ID3D11UnorderedAccessView* GetUAV() const { return m_UAV; }

		ENGINE_DLL inline void SetInitializationData(void* data) 
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
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView* m_UAV = nullptr;

		unsigned int m_ByteSize;
		unsigned int m_ElementStride;
		unsigned int m_CreationFlags;
		unsigned int m_RefCount = 1;
	};

	class GfxBuffer
	{
		DELETE_COPY_CONSTRUCTOR(GfxBuffer);
	public:
		ENGINE_DLL GfxBuffer(unsigned int byteSize, unsigned int creationFlags = 0, unsigned int elementStride = 0) :
			m_BufferResource(new GfxBufferResource(byteSize, creationFlags, elementStride)) {}

		ENGINE_DLL GfxBuffer(GfxBuffer* buffer):
			m_BufferResource(buffer->GetBufferResource())
		{
			m_BufferResource->AddRef();
		}

		ENGINE_DLL ~GfxBuffer()
		{
			m_BufferResource->Release();
		}

		inline GfxBufferResource* GetBufferResource() const { return m_BufferResource; }

	protected:
		GfxBufferResource* m_BufferResource;
	};

	class GfxVertexBuffer : public GfxBuffer
	{
		DELETE_COPY_CONSTRUCTOR(GfxVertexBuffer);
	public:
		struct VBData
		{
			void* pData;
			unsigned int numBytes;
			unsigned int stride;
		};

		ENGINE_DLL GfxVertexBuffer(const VBData data):
			GfxBuffer(data.numBytes, BCF_VertexBuffer | BCF_Usage_Immutable),
			m_NumVerts(data.numBytes/data.stride),
			m_Stride(data.stride),
			m_Offset(0)
		{
			m_BufferResource->SetInitializationData(data.pData);
		}

		inline unsigned int GetStride() const { return m_Stride; }
		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumVerts() const { return m_NumVerts; }

	private:
		unsigned int m_Stride = 0;
		unsigned int m_Offset = 0;
		unsigned int m_NumVerts = 0;
	};

	class GfxIndexBuffer : public GfxBuffer
	{
		DELETE_COPY_CONSTRUCTOR(GfxIndexBuffer);
	public:
		ENGINE_DLL GfxIndexBuffer(unsigned int* pIndices, unsigned int numIndices) :
			GfxBuffer(numIndices * sizeof(unsigned int), BCF_IndexBuffer | BCF_Usage_Immutable),
			m_NumIndices(numIndices),
			m_Offset(0) 
		{
			m_BufferResource->SetInitializationData(pIndices);
		}

		inline unsigned int GetOffset() const { return m_Offset; }
		inline unsigned int GetNumIndices() const { return m_NumIndices; }

	private:
		unsigned int m_Offset = 0;
		unsigned int m_NumIndices = 0;
	};

	template<typename T>
	class GfxConstantBuffer : public GfxBuffer
	{
		DELETE_COPY_CONSTRUCTOR(GfxConstantBuffer);
	public:

		GfxConstantBuffer<T>():
			GfxBuffer(sizeof(T) + 0xf & 0xfffffff0, BCF_ConstantBuffer | BCF_Usage_Dynamic | BCF_CPUWrite)
		{ }

		void Upload(const T& data)
		{
			if (!m_BufferResource->Initialized())
				m_BufferResource->Initialize();
			m_BufferResource->Upload(&data, sizeof(T));
		}
	};

	template<typename T>
	class GfxStructuredBuffer : public GfxBuffer
	{
		DELETE_COPY_CONSTRUCTOR(GfxStructuredBuffer);
	public:
		GfxStructuredBuffer(unsigned int numElements) :
			GfxBuffer(sizeof(T) * numElements, BCF_StructuredBuffer | BCF_Usage_Dynamic | BCF_CPUWrite, sizeof(T)),
			m_NumElements(numElements) {}

		void Upload(const T& data, unsigned int index)
		{
			ASSERT(index < m_NumElements, "Structured buffer overflow!");

			if (!m_BufferResource->Initialized())
				m_BufferResource->Initialize();
			m_BufferResource->Upload(&data, sizeof(T), index * sizeof(T));
		}

	private:
		unsigned int m_NumElements;

		ID3D11ShaderResourceView* m_Srv;
	};
}

#include "GfxBuffers.hpp"