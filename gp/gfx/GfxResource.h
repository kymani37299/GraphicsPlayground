#pragma once

#include "gfx/GfxCommon.h"

struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace GP
{
	enum ResourceCreationFlags
	{
		// Bindings
		RCF_SRV		=	1 << 0,		// Shader Resource View
		RCF_UAV		=	1 << 1,		// Unordered Access View
		RCF_RT		=	1 << 2,		// Render Target
		RCF_DS		=	1 << 3,		// Depth Stencil
		RCF_VB		=	1 << 4,		// Vertex Buffer
		RCF_IB		=	1 << 5,		// Index Buffer
		RCF_CB		=	1 << 6,		// Constant Buffer
		
		// Access flags
		RCF_CPURead	=	1 << 7,
		RCF_CPUWrite	=	1 << 8,
		RCF_CopyDest	=	1 << 9,

		// Misc
		RCF_GenerateMips		=	1 << 10,
		RCF_Cubemap				=	1 << 11,
		RCF_StructuredBuffer	=	1 << 12,

		// Combined
		RCF_CPUReadWrite = RCF_CPURead | RCF_CPUWrite,
	};

	enum class ResourceType
	{
		Invalid = 0,

		// Buffer
		VertexBufer,
		IndexBuffer,
		ConstantBuffer,
		StructuredBuffer,

		// Texture
		Texture2D,
		TextureArray2D,
		Cubemap,
		Texture3D
	};

	struct PathInitData
	{
		static constexpr unsigned int MAX_NUM_PATHS = 6;
		unsigned int numPaths = 0;
		std::string paths[MAX_NUM_PATHS];
	};

	struct MemoryInitData
	{
		unsigned int numBytes = 0;
		void* data = nullptr;
	};

	template<typename HandleType>
	class GfxResourceHandle
	{
		DELETE_COPY_CONSTRUCTOR(GfxResourceHandle);
	public:
		GfxResourceHandle(unsigned int creationFlags):
			m_CreationFlags(creationFlags) { }

		~GfxResourceHandle() { }

		inline bool Initialized() const { return m_Handle != nullptr; }

		inline void AddRef()
		{
			m_RefCount++;
		}

		inline void Release()
		{
			m_RefCount--;
			if (m_RefCount == 0) delete this;
		}

		inline void AddCreationFlags(unsigned int creationFlags)
		{
			if (Initialized()) LOG("[Warning][GfxResourceHandle] Trying to add creation flags to already initialized resource!");
			m_CreationFlags |= creationFlags;
		}

		inline HandleType* GetHandle() const { return m_Handle; }
		inline unsigned int GetCreationFlags() const { return m_CreationFlags; }

		inline void SetInitializationData(unsigned int numPaths, std::string paths[])
		{
			ASSERT(!m_Handle, "[GfxResourceHandle] Trying to set initialization data to already initialized resource!");
			ASSERT(m_PathData.numPaths <= PathInitData::MAX_NUM_PATHS, "[GfxResourceHandle] Assert failed: numPaths < MAX_NUM_PATHS");

			m_PathData.numPaths = numPaths;
			for (unsigned int i = 0; i < numPaths; i++)
			{
				m_PathData.paths[i] = paths[i];
			}
		}

		inline void SetInitializationData(void* data, unsigned int numBytes)
		{
			ASSERT(!m_Handle, "[GfxResourceHandle] Trying to set initialization data to already initialized resource!");

			m_MemData.numBytes = numBytes;
			m_MemData.data = malloc(numBytes);
			memcpy(m_MemData.data, data, numBytes);
		}

	protected:
		HandleType* m_Handle = nullptr;
		unsigned int m_CreationFlags;
		unsigned int m_RefCount = 1;

		// TODO: Put this 2 in union
		PathInitData m_PathData;
		MemoryInitData m_MemData;
	};

	template<typename ResourceHandle>
	class GfxResource
	{
		DELETE_COPY_CONSTRUCTOR(GfxResource);
	protected:
		GfxResource(ResourceType type):
		m_Type(type) {}

		GfxResource(ResourceType type, ResourceHandle* resource) :
			m_Type(type),
			m_Resource(resource)
		{
			m_Resource->AddRef();
		}

	public:
		inline bool Initialized() const
		{
			const unsigned int creationFlags = m_Resource->GetCreationFlags();
			bool srvOK = m_SRV || !(creationFlags & RCF_SRV);
			bool uavOK = m_UAV || !(creationFlags & RCF_UAV);
			return m_Resource->Initialized() && srvOK && uavOK;
		}

		GP_DLL void Initialize();

		inline void SetInitializationData(void* data) { m_Resource->SetInitializationData(data); }

		inline ResourceType GetType() const { return m_Type; }
		inline ResourceHandle* GetResource() const { return m_Resource; }
		inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
		inline ID3D11UnorderedAccessView* GetUAV() const { return m_UAV; }

		inline void AddCreationFlags(unsigned int flags) { m_Resource->AddCreationFlags(flags); }

	protected:
		ResourceType m_Type = ResourceType::Invalid;
		ResourceHandle* m_Resource = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		ID3D11UnorderedAccessView* m_UAV = nullptr;
	};
}