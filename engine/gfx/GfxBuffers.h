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
		GfxBufferResource(unsigned int byteSize, unsigned int creationFlags = 0, unsigned int elementStride = 0):
			m_ByteSize(byteSize),
			m_CreationFlags(creationFlags),
			m_ElementStride(elementStride) {}

		void Initialize();

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
}

#include "GfxBuffers.hpp"