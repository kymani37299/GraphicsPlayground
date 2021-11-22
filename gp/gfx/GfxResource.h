#pragma once

#include "gfx/GfxCommon.h"

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

	protected:
		HandleType* m_Handle = nullptr;
		unsigned int m_CreationFlags;
		unsigned int m_RefCount = 1;
	};
}